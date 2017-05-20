#include "output_producer.h"


#include "debug_utils.h"
#include "global_variables.h"

#include "event_list.h"

#include "quicksort.h"
#include "utils.h"

#include <stdlib.h>
#include <stdio.h>

#include <mpi.h>

#include <limits.h>
#include <string.h>

#ifndef UNDEF_SIGN
  #define UNDEF_SIGN "-"
#endif


extern const int VALUED_EVENT_NUMBER_TAG;
extern const int VALUED_EVENT_TRANSMISSION_TAG;
extern const int VALUED_EVENT_PROBE_TAG;
extern const int VALUED_EVENT_TS_TAG;

char * to_string_top_three(valued_event * tt, int ts);

int produce_output_file(char *output_file_name, int group_size, MPI_Datatype mpi_valued_event, int start_ts)
{
  MPI_Status ret;
  //int ts=1265001000;
  //int ts=1264993-100;
  int ts=start_ts-100;
  print_info("Start ts: %d", ts);
  unsigned int worker_terminated_mask=0;
  int active_workers=group_size-1;
  //probe workers that can transmit something
  for(int worker=1; worker<group_size;worker++)
  {
    unsigned int probe;
    MPI_Recv(&probe,1,MPI_UNSIGNED,worker,VALUED_EVENT_PROBE_TAG*worker,MPI_COMM_WORLD,&ret );
    if(probe==0)
    {
      worker_terminated_mask|= 1<< (worker-1);
      active_workers--;
    }
  }
  valued_event current_top_three[3];
  for(int i=0; i<3;i++)
  {
     valued_event * ve_buf=new_valued_event(-1,-1,-1,-1,-1);
    current_top_three[i]=*ve_buf;
    clear_valued_event(ve_buf);
  }

  //now we produce the output
  FILE * out_fp = fopen(output_file_name,"w"); 
  if(out_fp==NULL)
  {
    print_error("Cannot open output file at path %s", output_file_name);
    return -1;
  }

  while( worker_terminated_mask < (1<<group_size-1)-1 && active_workers>0 )
  {
    //check every one week
    if(ts%(24*60*60*7)==0)
    {
      print_fine("Master is handling timestamp %d", ts);
    } 
    ts++;
    int ts_buf=ts;
    //receive valued_events arrays.
    valued_event ** ve_matrix=malloc(sizeof(valued_event*)*active_workers);
    int * ve_matrix_dim=malloc(sizeof(valued_event*)*active_workers);
    int ve_matrix_size=0;
    //print_fine("--------------------------ts: %d",ts);
    MPI_Bcast(&ts_buf,1,MPI_INT,MPI_MASTER,MPI_COMM_WORLD);
    for(int i=0; i<group_size-1;i++)
    {
      int rec_size;
      if( ( (worker_terminated_mask >> i) & 1 )==0 )
      {
        MPI_Recv(&rec_size,1,MPI_INT,i+1,VALUED_EVENT_NUMBER_TAG*(i+1),MPI_COMM_WORLD,&ret);
        //print_fine("Received rec_size of %d from worker %d for ts %d", rec_size, i+1,ts);
        //remove worker from active ones.
        if(rec_size<0)
        {
          worker_terminated_mask|= 1U<<i;
          print_info("Master removed worker %d publishers",i+1);
          active_workers--; 
        }
        else
        {
          //add valued_event array to ve_matrix
          if(rec_size>0)
          {
            ve_matrix_dim[ve_matrix_size]=rec_size;
            //print_fine("rec_size: %d", rec_size);
            valued_event * ve_ar=malloc(sizeof(valued_event)*rec_size);
            if(ve_ar==NULL)
            {
              //print_error("Cannot malloc valued_events array in reception of worker %d", i+1);
              return -1;
            }
            //print_fine("master is w8ing for ve_ar reception from %d for ts %d",i+1,ts);
            MPI_Recv(ve_ar,rec_size,mpi_valued_event,i+1,VALUED_EVENT_TRANSMISSION_TAG*(i+1), MPI_COMM_WORLD, &ret);
            //print_fine("Master received valued_events of size %d from worker %d for ts %d. put array at position %d of ve_matrix", rec_size, i+1,ts, ve_matrix_size);
            ve_matrix[ve_matrix_size]=ve_ar;
            ve_matrix_size++;
          }
        }
      }
    }
    //print_fine("combining for ts %d",ts);
    //print_fine("ve_matrix_size: %d", ve_matrix_size);
    if(ve_matrix_size==0)
    {
      free(ve_matrix);
      free(ve_matrix_dim);
      continue;
    }
    int global_ve_size;
    valued_event * ve_ar =merge_valued_event_array_score_ordered(ve_matrix, ve_matrix_dim, ve_matrix_size, &global_ve_size);
    //print_fine("produced ordered ve_ar");
    //ve _matrix is no longer needed
    for(int j=0; j<ve_matrix_size; j++)
    {
      free(ve_matrix[j]);
    }
    free(ve_matrix);
    free(ve_matrix_dim);
    //print_fine("master ve_ar for ts: %d", ts);
    /*for(int i=0; i<global_ve_size;i++)
    {
      print_valued_event(ve_ar+i);
    }*/
    valued_event * final_ve = malloc(sizeof(valued_event)*(3+global_ve_size));
    memcpy(final_ve,current_top_three,sizeof(valued_event)*3);
    int final_ve_size=3;
    //merge current_top_three and ve_ar into final_ve
    for(int i=0; i<global_ve_size; i++ )
    {
      char changed=0;
      for(int j=0; changed==0 && j<3;j++)
      {
        if(final_ve[j].post_id==ve_ar[i].post_id)
        {
          final_ve[j]=ve_ar[i];
          changed=1;
        }
      }
      if(changed==0)
      {
        final_ve[final_ve_size]=ve_ar[i];
        final_ve_size++;
      }
    }
    sort_valued_events_on_score_with_array(final_ve,0,final_ve_size-1);
    /*print_fine("created final_ve. Printing it");
    for(int i=0; i<final_ve_size; i++)
    {
      print_valued_event(final_ve+i);
    }*/
    char changed=0;
    for(int i=0; changed==0 &&  i<3;i++)
    {
      if(final_ve[i].post_id!=current_top_three[i].post_id ||
        final_ve[i].score!=current_top_three[i].score ||
        final_ve[i].n_commenters!=current_top_three[i].n_commenters)
      {
        changed=1;
      }
    }
    if(changed==1)
    {
      //print_fine("writin output at ts: %d", ts);
      char * output_line = to_string_top_three(final_ve, ts);
      //print_info("writing %s to output_line", output_line);
      fprintf(out_fp, "%s\n", output_line);
      free(output_line);
      memcpy(current_top_three,final_ve,sizeof(valued_event)*3);
    }
    free(ve_ar);
    free(final_ve);
  }
  fclose(out_fp);
  return 0;
}


char * to_string_top_three(valued_event * tt, int ts)
{
  if(tt==NULL)
  {
    print_warning("Passed NULL to print_top_three. returning empty string");
    char * ret_empty = malloc(sizeof(char));
    ret_empty[0]='\0';
    return ret_empty;
  }
  char * output = malloc(sizeof(char)*1024);
  if(output==NULL)
  {
    print_error("Cannot malloc array for to_string_tuple_top_three");
  }
  char buf[1024];
  output[0]='\0';
  itoa(ts, buf);
  strcat(output,buf);
  for(int i=0; i<3; i++)
  {
    strcat(output,",");
    if(tt[i].score<=0)
    {
      strcat(output,"-,-,-,-");
    }
    else{
      if(tt[i].post_id==-1)
      {
        strcat(output,UNDEF_SIGN);
      }
      else
      {
        ltoa(tt[i].post_id,buf);
        strcat(output,buf);
      }
      strcat(output,",");
      if(tt[i].user_id==-1)
      {
        strcat(output,UNDEF_SIGN);
      }
      else
      {
        ltoa(tt[i].user_id,buf);
        strcat(output,buf);
      }
      strcat(output, ",");
      if(tt[i].score==-1)
      {
        strcat(output,UNDEF_SIGN);
      }
      else
      {
        itoa(tt[i].score,buf);
        strcat(output,buf);
      }
      strcat(output, ",");
      if(tt[i].score==-1)
      {
        strcat(output,UNDEF_SIGN);
      }
      else
      {
        itoa(tt[i].n_commenters,buf);
        strcat(output,buf);
      }
    }
  }
  return output;
}
