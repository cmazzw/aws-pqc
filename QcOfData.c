#include "./include/sqlite3.h"
#include <stdio.h>
#include <stdlib.h>
#include<string.h>

#define LEN_DATA sizeof(sta_data)

typedef struct station_data
{
   double tem_test;
   char station_num[6];//台站号
   char o_time[15];//观测时间
   char wind2[4];//两分钟平均风速
   char wind10[4];//十分钟平均风速
   char wind_max[4];//最大风速
   char wind_s[4];
   char wind_j[4];
   char rain[5];//小时降水量
   char tem[5];//气温
   char tem_max[5];//最高气温
   char tem_min[5];//最低气温
   char pres[6];//本站气压
   char pres_max[6];
   char pres_min[6];
   char g_temp[5];
   char g_temp_max[5];
   char g_temp_min[5];
   char s_pres[6];//海平面气压
   char filename[512];//数据所在文件名------这里注意，赋值越界了。
   struct station_data *next;
}sta_data;


typedef struct qc_data
{
   char mon[3];
   char wind_min[4];
   char wind_max[4];
   char rain_min[5];
   char rain_max[5];
   char temp_min[5];
   char temp_max[5];
   char pres_min[6];
   char pres_max[6];
   char g_temp_min[5];
   char g_temp_max[5];
   char s_pres_min[6];
   char s_pres_max[6];
}qc;

int substr(char dst[], char src[],int start, int len)  
{  
    int i;  
    for(i=0;i<len;i++)  
    {  
        dst[i]=src[start+i];    //从第start+i个元素开始向数组内赋值  
    }  
    dst[i]='\0';  
    return i;  
}  

int getqcbymon(qc *qc_st,const char *mon)
{
  sqlite3 *db=NULL;
  int rc;
  //打开指定的数据库文件,如果不存在将创建一个同名的数据库文件  
  rc = sqlite3_open("/home/acenter/src/rqc/testDB.db", &db);
  if(rc)
  {
    fprintf(stderr, "Can't open database: %s/n", sqlite3_errmsg(db));
    sqlite3_close(db);
    return 0;
  }
  char sql[256];
  sprintf(sql,"select * FROM qc WHERE MON='%s'",mon);
  char **data;
  char *errmsg=NULL;
  int nRow, nColumn;
  int ret = sqlite3_get_table(db,sql,&data,&nRow,&nColumn,&errmsg);
  int i,j,index;
  if(ret == SQLITE_OK)
    {
       //输出数据
       index=nColumn;
       for(i=0;i<nRow;i++)
        {
          for(j=0;j<nColumn;j++)
            {
               if(index==nColumn)
                  strcpy(qc_st->mon,data[index]);
               if(index==(nColumn+1))
                  strcpy(qc_st->wind_min,data[index]);
               if(index==(nColumn+2))
                  strcpy(qc_st->wind_max,data[index]);
               if(index==(nColumn+3))
                  strcpy(qc_st->rain_min,data[index]);
               if(index==(nColumn+4))
                  strcpy(qc_st->rain_max,data[index]);
               if(index==(nColumn+5))
                  strcpy(qc_st->temp_min,data[index]);
               if(index==(nColumn+6))
                  strcpy(qc_st->temp_max,data[index]);
               if(index==(nColumn+7))
                  strcpy(qc_st->pres_min,data[index]);
               if(index==(nColumn+8))
                  strcpy(qc_st->pres_max,data[index]);
               if(index==(nColumn+9))
                  strcpy(qc_st->g_temp_min,data[index]);
               if(index==(nColumn+10))
                  strcpy(qc_st->g_temp_max,data[index]);
               if(index==(nColumn+11))
                  strcpy(qc_st->s_pres_min,data[index]);
               if(index==(nColumn+12))
                  strcpy(qc_st->s_pres_max,data[index]);
               index++;
            }
        }
    }
  sqlite3_free_table(data);
  sqlite3_close(db); //关闭数据库*/
  return 1;
}


sta_data *qc_fun(sta_data *pNode)
{
    sta_data *pMove;
    sta_data *pMovePre;
    pMovePre = pNode;
    pMove = pNode->next;
    while (pMove != NULL) 
     {
        //对pMove节点进行质量检查
        //获取月份
        char mon[3];
        int m;  
        m=substr(mon,pMove->o_time,4,2);
        //printf("mon is %s\n",mon);
        //根据时次中的月份获取对应要素的范围
        struct qc_data qc_st;
        getqcbymon(&qc_st,mon);
            
        int qc_flag=1; 

        double wind2=atof(pMove->wind2)/10;
        double wind10=atof(pMove->wind10)/10;
        double wind_max=atof(pMove->wind_max)/10;
        double wind_s=atof(pMove->wind_s)/10;
        double wind_j=atof(pMove->wind_j)/10;

        double rain=atof(pMove->rain)/10;

        char tem[4],zf_flag[2];
        substr(tem,pMove->tem,1,3);
        substr(zf_flag,pMove->tem,0,1);
        double temprature;
        if(zf_flag[0]=='0')
          temprature=atof(tem)/10;
        else if(zf_flag[0]=='1')
          temprature=atof(tem)/10*(-1);
       

        char tem_max[4],zf_flag_max[2];
        substr(tem_max,pMove->tem_max,1,3);
        substr(zf_flag_max,pMove->tem_max,0,1);
        double temprature_max;
        if(zf_flag_max[0]=='0')
          temprature_max=atof(tem_max)/10;
        else if(zf_flag_max[0]=='1')
          temprature_max=atof(tem_max)/10*(-1); 

        char tem_min[4],zf_flag_min[2];
        substr(tem_min,pMove->tem_min,1,3);
        substr(zf_flag_min,pMove->tem_min,0,1);
        double temprature_min;
        if(zf_flag_min[0]=='0')
          temprature_min=atof(tem_min)/10;
        else if(zf_flag_min[0]=='1')
          temprature_min=atof(tem_min)/10*(-1);

        double pres=atof(pMove->pres)/10;
        double pres_min=atof(pMove->pres_min)/10;
        double pres_max=atof(pMove->pres_max)/10;

        char g_temp[4],g_temp_flag[2];
        substr(g_temp,pMove->g_temp,1,3);
        substr(g_temp_flag,pMove->g_temp,0,1);
        double temprature_g_temp;
        if(g_temp_flag[0]=='0')
          temprature_g_temp=atof(g_temp)/10;
        else if(g_temp_flag[0]=='1')
          temprature_g_temp=atof(g_temp)/10*(-1);

        char g_temp_max[4],g_temp_max_flag[2];
        substr(g_temp_max,pMove->g_temp_max,1,3);
        substr(g_temp_max_flag,pMove->g_temp_max,0,1);
        double temprature_g_temp_max;
        if(g_temp_max_flag[0]=='0')
          temprature_g_temp_max=atof(g_temp_max)/10;
        else if(g_temp_max_flag[0]=='1')
          temprature_g_temp_max=atof(g_temp_max)/10*(-1);

        char g_temp_min[4],g_temp_min_flag[2];
        substr(g_temp_min,pMove->g_temp_min,1,3);
        substr(g_temp_min_flag,pMove->g_temp_min,0,1);
        double temprature_g_temp_min;
        if(g_temp_min_flag[0]=='0')
          temprature_g_temp_min=atof(g_temp_min)/10;
        else if(g_temp_min_flag[0]=='1')
          temprature_g_temp_min=atof(g_temp_min)/10*(-1);

        double s_pres=atof(pMove->s_pres)/10;

        printf("%s->%f\n",pMove->tem,temprature);

        printf("%s->%f\n",pMove->wind2,wind2);       
        if (strcmp(pMove->station_num,"88888")==0) 
         {
            pMovePre->next = pMove->next;
            free(pMove);
            pMove = pMovePre->next;
            continue;
         }
        pMovePre = pMovePre->next;
        pMove = pMove->next;
    }
    return pNode;
}


int create(sta_data *node)
{
   node->tem_test=5;
   sprintf(node->station_num,"H0414");
   sprintf(node->o_time,"20180417000000");
   sprintf(node->wind2,"112");
   sprintf(node->wind10,"105");
   sprintf(node->wind_max,"109");
   sprintf(node->wind_s,"103");
   sprintf(node->wind_j,"156");
   sprintf(node->rain,"0000");
   sprintf(node->tem,"1140");
   sprintf(node->tem_max,"0140");
   sprintf(node->tem_min,"0130");
   sprintf(node->pres,"/////");
   sprintf(node->pres_max,"/////");
   sprintf(node->pres_min,"/////");
   sprintf(node->g_temp,"////");
   sprintf(node->g_temp_max,"////");
   sprintf(node->g_temp_min,"////");
   sprintf(node->g_temp_min,"/////");
   sprintf(node->filename,"Z_SURF_C_BEHB-REG_20180417000029_O_AWS_FTM.txt");
}

sta_data *init()
{
   sta_data *head,*p,*node;
   head=node=(sta_data*)malloc(LEN_DATA);
   int i=0,size=3;
   for(i=0;i<size;i++)
    {
       p=node;
       node=(sta_data*)malloc(LEN_DATA);
       create(node);
       p->next=node;
    }
   node->next=NULL;
   return(head);
}

void free_list(sta_data *head)
{
    sta_data *pointer;
    while(head!=NULL)
    {
       pointer = head;
       head = head->next;
       free(pointer);
       pointer=NULL;
    }
}


int main()
{
    sta_data *head=init();
    //格式检查后的输出
    int num=1;
    sta_data *tmp=head;
    tmp=tmp->next;
    while(tmp!=NULL)
      {
          printf("这是第%d个节点，数据为:station:%s,time:%s,wind2:%s,filename:%s\n",num,tmp->station_num,tmp->o_time,tmp->wind2,tmp->filename);
          tmp=tmp->next;
          num++;
      }
    printf("=============================\n");
   
    //质量控制
    sta_data *delete=head;
    qc_fun(delete);


    //质量控制后的输出
    num=1;
    tmp=head;
    tmp=tmp->next;
    while(tmp!=NULL)
      {
          printf("这是第%d个节点，数据为:station:%s,time:%s,wind2:%s,filename:%s\n",num,tmp->station_num,tmp->o_time,tmp->wind2,tmp->filename);
          tmp=tmp->next;
          num++;
      }    
}
