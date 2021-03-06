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

extern const int TOP_NUMBER;

char * to_string_top_three(valued_event * tt, int ts);

int produce_output_file(char *output_file_name, int group_size, MPI_Datatype mpi_valued_event, int start_ts)
{
    MPI_Status ret;
    int ts=start_ts-100;
    print_info("Start ts: %d", ts);
    char * worker_terminated_mask=calloc(sizeof(char),group_size-1);
    int active_workers=group_size-1;
    print_fine("active_workers: %d",active_workers);
    int top_score_array_size=TOP_NUMBER;
    valued_event *  top_score_array=malloc(sizeof(valued_event)*top_score_array_size);
    for(int i=0; i<TOP_NUMBER; i++)
    {
        valued_event * ve_buf=new_dummy_valued_event();
        top_score_array[i]=*ve_buf;
        clear_valued_event(ve_buf);
    }

    //now we produce the output
    FILE * out_fp = fopen(output_file_name,"w");
    if(out_fp==NULL)
    {
        print_error("Cannot open output file at path %s", output_file_name);
        return -1;
    }

    while( active_workers>0 )
    {
        //check every 28 days
        if(ts%(24*60*60*7*4)==0)
        {
            print_fine("Master is handling timestamp %d", ts);
        }
        ts++;
        int ts_buf=ts;
        //receive valued_events arrays.
        valued_event ** ve_matrix=malloc(sizeof(valued_event*)*active_workers);
        int * ve_matrix_dim=malloc(sizeof(valued_event*)*active_workers);
        int ve_matrix_size=0;
        for(int i=0; i<group_size-1; i++)
        {
            int rec_size;
            if( worker_terminated_mask[i]==0 )
            {
                MPI_Send(&ts,1,MPI_INT,i+1,VALUED_EVENT_TS_TAG*(i+1),MPI_COMM_WORLD);
                MPI_Recv(&rec_size,1,MPI_INT,i+1,VALUED_EVENT_NUMBER_TAG*(i+1),MPI_COMM_WORLD,&ret);
                //remove worker from active ones.
                if(rec_size<0)
                {
                    worker_terminated_mask[i]=1;
                    active_workers--;
                    print_info("Master removed worker %d at ts %d. active_workers: %d",i+1,ts,active_workers);
                    continue;
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
                            print_error("Cannot malloc valued_events array in reception of worker %d", i+1);
                            return -1;
                        }
                        //print_fine("master is w8ing for ve_ar reception from %d for ts %d",i+1,ts);
                        MPI_Recv(ve_ar,rec_size,mpi_valued_event,i+1,VALUED_EVENT_TRANSMISSION_TAG*(i+1), MPI_COMM_WORLD, &ret);
                        ve_matrix[ve_matrix_size]=ve_ar;
                        ve_matrix_size++;
                    }
                }
            }
        }
        if(ve_matrix_size==0)
        {
            free(ve_matrix);
            free(ve_matrix_dim);
            continue;
        }
        int global_ve_size;
        valued_event * ve_ar =merge_valued_event_array_score_ordered(ve_matrix, ve_matrix_dim, ve_matrix_size, &global_ve_size);
        //ve _matrix is no longer needed
        for(int j=0; j<ve_matrix_size; j++)
        {
            free(ve_matrix[j]);
        }
        free(ve_matrix);
        free(ve_matrix_dim);
        /*print_fine("master ve_ar for ts: %d", ts);
        for(int i=0; i<global_ve_size;i++)
        {
          print_valued_event(ve_ar+i);
        }*/
        valued_event * final_ve = malloc(sizeof(valued_event)*(top_score_array_size+global_ve_size));
        memcpy(final_ve,top_score_array,sizeof(valued_event)*top_score_array_size);
        int final_ve_size=top_score_array_size;
        //merge top_score_array and ve_ar into final_ve
        for(int i=0; i<global_ve_size; i++ )
        {
            char changed=0;
            for(int j=0; changed==0 && j<top_score_array_size; j++)
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
        final_ve = realloc(final_ve,sizeof(valued_event)*final_ve_size);
        if(final_ve==NULL)
        {
            print_fine("Cannot realloc final_ve");
        }
        sort_valued_events_on_score_with_array(final_ve,0,final_ve_size-1);
        /*print_fine("created final_ve for ts %d. Printing it", ts);
        for(int i=0; i<final_ve_size; i++)
        {
          print_valued_event(final_ve+i);
        }*/
        char changed=0;
        char all_zero=1;
        for(int i=0; changed==0 &&  i<TOP_NUMBER; i++)
        {
            if(final_ve[i].post_id!=top_score_array[i].post_id ||
                    final_ve[i].score!=top_score_array[i].score ||
                    final_ve[i].n_commenters!=top_score_array[i].n_commenters)
            {
                changed=1;
            }
            if(all_zero==1 && final_ve[i].score>0)
            {
                all_zero=0;
            }
        }
        if(changed==1 && all_zero!=1)
        {
            //print_fine("writin output at ts: %d", ts);
            char * output_line = to_string_top_three(final_ve, ts);
            //print_info("writing %s to output_line", output_line);
            fprintf(out_fp, "%s\n", output_line);
            free(output_line);

            int lowest_important_score = final_ve[TOP_NUMBER-1].score;
            int score_counter=TOP_NUMBER-1;
            while(score_counter<final_ve_size && lowest_important_score>0 && lowest_important_score==final_ve[score_counter].score)
            {
                score_counter++;
            }
            top_score_array= realloc(top_score_array,sizeof(valued_event)*score_counter);
            if(top_score_array==NULL)
            {
                print_error("error in top_score_array realloc");
                return -1;
            }
            memcpy(top_score_array,final_ve,sizeof(valued_event)*score_counter);
            top_score_array_size = score_counter;
            if(top_score_array_size<TOP_NUMBER)
            {
                top_score_array=realloc(top_score_array,sizeof(valued_event)*TOP_NUMBER);
                while(top_score_array_size<TOP_NUMBER)
                {
                    valued_event * buf = new_dummy_valued_event();
                    top_score_array[top_score_array_size]=*buf;
                    clear_valued_event(buf);
                    top_score_array_size++;
                }
            }
        }
        free(ve_ar);
        free(final_ve);
    }
    free(worker_terminated_mask);
    free(top_score_array);
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
    for(int i=0; i<TOP_NUMBER; i++)
    {
        strcat(output,",");
        if(tt[i].score<=0)
        {
            strcat(output,"-,-,-,-");
        }
        else
        {
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
