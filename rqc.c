#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <sqlite3.h>

#define LEN_DATA sizeof(sta_data)

struct qc_result
{
    char station_num[6];
    char o_time[15];
    char ele[16];
    double ele_value;
    char filename[64];
};

typedef struct qc_threshold
{
   char mon[3];
   double wind_min;
   double wind_max;
   double rain_min;
   double rain_max;
   double temp_min;
   double temp_max;
   double pres_min;
   double pres_max;
   double g_temp_min;
   double g_temp_max;
   double s_pres_min;
   double s_pres_max;
}threshold;

struct station_temp
{
    char station_num[6];//台站号   H0414
    char o_time[15];//观测时间     20180417000000
    char wind2[4];//两分钟平均风速  112
    char wind10[4];//十分钟平均风速 105
    char wind_max[4];//最大风速     109
    char wind_s[4];
    char wind_j[4];
    char rain[5];//小时降水量0000
    char tem[5];//气温0140
    char tem_max[5];//最高气温0140
    char tem_min[5];//最低气温0130
    char pres[6];//本站气压   /////
    char pres_max[6];         /////
    char pres_min[6];
    char g_temp[5];
    char g_temp_max[5];
    char g_temp_min[5];
    char s_pres[6];//海平面气压
    char first[35];
    char second[263];
    char three[122];
    char four[136];
    char filename[64];
};

typedef struct station_data
{
   char station_num[6];//台站号   H0414
   char o_time[15];//观测时间     20180417000000
   double wind2;//两分钟平均风速  112
   double wind10;//十分钟平均风速 105
   double wind_max;//最大风速     109
   double wind_s;
   double wind_j;
   double rain;//小时降水量0000
   double tem;//气温0140
   double tem_max;//最高气温0140
   double tem_min;//最低气温0130
   double pres;//本站气压   /////
   double pres_max;         /////
   double pres_min;
   double g_temp;
   double g_temp_max;
   double g_temp_min;
   double s_pres;//海平面气压
   char first[35];
   char second[263];
   char three[122];
   char four[136];
   char filename[64];//数据所在文件名------这里注意，赋值越界了。
   struct station_data *next;
}sta_data;

int match(char *ptext,char *name)
{
   if(*name=='\0')
   {
      return(1);
   }
   if(*ptext=='\0')
   {
       if(*name=='*'&&*(name+1)=='\0')
         {
            return(1);
         }
       return(0);
   }
   if(*name!='*'&&*name!='?')
   {
       if(*name==*ptext)
          return  match(ptext+1,name+1);
       return(0);
   }
   if(*name=='*')
      return match(ptext+1,name)||match(ptext,name+1);
   return match(ptext+1,name+1);
}

int reglen(char *s)
{
    int num=0;
    int i = 0;      
    while(s[i]!='\0')
      {
        if(s[i]>='a'&&s[i]<='z')
          num++;
        else if(s[i]>='A'&&s[i]<='Z')
          num++;
        else if(s[i]=='/' || s[i]=='-' || s[i]==' ' || s[i]=='=')
          num++;
        else if(s[i]>='0'&&s[i]<='9')
          num++;
        i++;
      }
    return num;
}

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

//对于缺测"////"则转换为特征值999.999
int strtodouble(sta_data *pTail,struct station_temp *sta_temp)
{
    sprintf(pTail->station_num,sta_temp->station_num);
    sprintf(pTail->o_time,sta_temp->o_time);

    //两分钟风速
    if(sta_temp->wind2[0]=='/')
        pTail->wind2=999.999;
    else
        pTail->wind2=atof(sta_temp->wind2)/10;

    //十分钟风速
    if(sta_temp->wind10[0]=='/')
        pTail->wind10=999.999;
    else
        pTail->wind10=atof(sta_temp->wind10)/10;

    //最大风速
    if(sta_temp->wind_max[0]=='/')
        pTail->wind_max=999.999;
    else
        pTail->wind_max=atof(sta_temp->wind_max)/10;

    //瞬时风速
    if(sta_temp->wind_s[0]=='/')
        pTail->wind_s=999.999;
    else
        pTail->wind_s=atof(sta_temp->wind_s)/10;

    //极大风速
    if(sta_temp->wind_j[0]=='/')
        pTail->wind_j=999.999;
    else
        pTail->wind_j=atof(sta_temp->wind_j)/10;

    //降雨量数据
    if(sta_temp->rain[0]=='/')
        pTail->rain=999.999;
    else
        pTail->rain=atof(sta_temp->rain)/10;

    //温度
    if(sta_temp->tem[0]=='/')
        pTail->tem=999.999;
    else
      {
         char temp[4],zf_flag[2];
         substr(temp,sta_temp->tem,1,3);
         substr(zf_flag,sta_temp->tem,0,1);
         if(zf_flag[0]=='0')
            pTail->tem=atof(temp)/10;
         else
            pTail->tem=atof(temp)/10*(-1);
      }

    //最大温度
    if(sta_temp->tem_max[0]=='/')
       pTail->tem_max=999.999;
    else
      {
         char temp_max[4],zf_flag_max[2];
         substr(temp_max,sta_temp->tem_max,1,3);
         substr(zf_flag_max,sta_temp->tem_max,0,1);
         if(zf_flag_max[0]=='0')
            pTail->tem_max=atof(temp_max)/10;
         else
            pTail->tem_max=atof(temp_max)/10*(-1);
      }

    //最低温度
    if(sta_temp->tem_min[0]=='/')
       pTail->tem_min=999.999;
    else
      {
         char temp_min[4],zf_flag_min[2];
         substr(temp_min,sta_temp->tem_min,1,3);
         substr(zf_flag_min,sta_temp->tem_min,0,1);
         if(zf_flag_min[0]=='0')
            pTail->tem_min=atof(temp_min)/10;
         else
            pTail->tem_min=atof(temp_min)/10*(-1);
      }

    //本站气压
    if(sta_temp->pres[0]=='/')
       pTail->pres=999.999;
    else
       pTail->pres=atof(sta_temp->pres)/10;

    //最低气压
    if(sta_temp->pres_min[0]=='/')
       pTail->pres_min=999.999;
    else
       pTail->pres_min=atof(sta_temp->pres_min)/10;

    //最高气压
    if(sta_temp->pres_max[0]=='/')
       pTail->pres_max=999.999;
    else
       pTail->pres_max=atof(sta_temp->pres_max)/10;

    //草面温度
    if(sta_temp->g_temp[0]=='/')
       pTail->g_temp=999.999;
    else
     {
       char g_temp_t[4],g_temp_flag[2];
       substr(g_temp_t,sta_temp->g_temp,1,3);
       substr(g_temp_flag,sta_temp->g_temp,0,1);
       if(g_temp_flag[0]=='0')
          pTail->g_temp=atof(g_temp_t)/10;
       else
          pTail->g_temp=atof(g_temp_t)/10*(-1);
     }

    //最高草面温度
    if(sta_temp->g_temp_max[0]=='/')
       pTail->g_temp_max=999.999;
    else
     {
       char g_temp_max_t[4],g_temp_max_flag[2];
       substr(g_temp_max_t,sta_temp->g_temp_max,1,3);
       substr(g_temp_max_flag,sta_temp->g_temp_max,0,1);
       if(g_temp_max_flag[0]=='0')
          pTail->g_temp_max=atof(g_temp_max_t)/10;
       else
          pTail->g_temp_max=atof(g_temp_max_t)/10*(-1);
     }

    //最低草面温度
    if(sta_temp->g_temp_min[0]=='/')
       pTail->g_temp_min=999.999;
    else
     {
       char g_temp_min_t[4],g_temp_min_flag[2];
       substr(g_temp_min_t,sta_temp->g_temp_min,1,3);
       substr(g_temp_min_flag,sta_temp->g_temp_min,0,1);
       if(g_temp_min_flag[0]=='0')
         pTail->g_temp_min=atof(g_temp_min_t)/10;
       else
         pTail->g_temp_min=atof(g_temp_min_t)/10*(-1);
     }

    //海平面气压
    if(sta_temp->s_pres[0]=='/')
       pTail->s_pres=999.999;
    else
       pTail->s_pres=atof(sta_temp->s_pres)/10;

    sprintf(pTail->first,sta_temp->first);
    sprintf(pTail->second,sta_temp->second);
    sprintf(pTail->three,sta_temp->three);
    sprintf(pTail->four,sta_temp->four);
    sprintf(pTail->filename,sta_temp->filename);
    return(1); 
}

//对于缺测"////"则记录特征值999.999
sta_data *readregfile(char *path,char *filename,FILE *log_fp)
{
    sta_data *pHead,*p,*pTail;

    pHead=pTail=(sta_data*)malloc(LEN_DATA);

    char fileallname[512];
    sprintf(fileallname,"%s/%s",path,filename);
    FILE* reg_fp = fopen(fileallname, "r");
    char line[300];
    int num=0;//记录当前读取的num行,前提是每组有四行
    int line_num=0;//记录真实的行数
    int group=0;//记录站数，也就是第几组

    struct station_temp sta_temp;
    
    while (fgets(line, sizeof(line), reg_fp))
      {
         line_num++;
         if(num-group*4==0)//解析第一行(基本信息)
            {
               if(reglen(line)==34)
                 {
                    int f_status=sscanf(line,"%5s",sta_temp.station_num);
                    strncpy(sta_temp.first,line,34);sta_temp.first[34]='\0';
                    if(f_status!=1)
                       {
                         //重新对错误数据进行异常值赋值，并提醒错误数据
                         sprintf(sta_temp.station_num,"99999");
                         fprintf(log_fp,"%s %d 基本信息字节数正确,格式错误\n",filename,line_num);
                       }
                 }
               else
                  {
                     sprintf(sta_temp.station_num,"99999");
                     strncpy(sta_temp.first,line,34);sta_temp.first[34]='\0';
                     fprintf(log_fp,"%s %d 基本信息行的字节数不正确\n",filename,line_num);
                  }
               num++;
               continue;
            }
         else if(num-group*4==1)//解析第二行(主要要素)
            {
               if(reglen(line)==262)
                 {
                   int s_status=sscanf(line,"%14s %*s %3s %*s %3s %*s %3s %*s %*s %3s \
                   %*s %3s %*s %4s %4s %4s %*s %4s %*s %*s %*s %*s %*s %*s %5s %5s %*s %5s %*s %4s \
                   %4s %*s %4s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %5s %*s %*s %*s", \
                   sta_temp.o_time,sta_temp.wind2,sta_temp.wind10,sta_temp.wind_max,sta_temp.wind_s,sta_temp.wind_j, \
                   sta_temp.rain,sta_temp.tem,sta_temp.tem_max,sta_temp.tem_min,sta_temp.pres,sta_temp.pres_max,sta_temp.pres_min, \
                   sta_temp.g_temp,sta_temp.g_temp_max,sta_temp.g_temp_min,sta_temp.s_pres);
                   strncpy(sta_temp.second,line,262);sta_temp.second[262]='\0';
                   if(s_status!=17)
                       {
                         //重新对错误数据进行异常值赋值，并提醒错误数据
                         sprintf(sta_temp.o_time,"99999999999999");
                         fprintf(log_fp,"%s %d 第二段观测数据字节数正确，格式错误\n",filename,line_num);
                       }
                 }
               else
                  {
                     //重新对错误数据进行异常值赋值，并提醒错误数据
                     sprintf(sta_temp.o_time,"99999999999999");
                     fprintf(log_fp,"%s %d 第二行观测数据的字节数不正确\n",filename,line_num);
                  }
               num++;
           }
         else if(num-group*4==2)//解析第三行(小时内分钟降水)
            {
               strncpy(sta_temp.three,line,121);sta_temp.three[121]='\0';
               num++;
           }
         else if(num-group*4==3)//解析第四行
            {
               group++;
               if(reglen(line)==34)
                 {
                    //读到了新的台站数据
                    //0、将第四行赋值为NNNN
                    strncpy(sta_temp.four,"NNNN",135);sta_temp.four[135]='\0';
                    strncpy(sta_temp.filename,filename,63);sta_temp.filename[63]='\0';
                    //1、先将上一个格式正确的台站数据写入
                    if(strcmp(sta_temp.station_num,"99999")!=0 && strcmp(sta_temp.o_time,"99999999999999")!=0)
                      {
                         //格式正确的数据写入
                         p=pTail;
                         pTail=(sta_data*)malloc(LEN_DATA);
                         strtodouble(pTail,&sta_temp);//将字符串格式转换为double格式
                         p->next=pTail;
                      }
                    //2、置空结构体
                    memset(&sta_temp,0,sizeof(struct station_temp));
                    //3、读取新的台站的第一行数据
                    int n_status=sscanf(line,"%5s",sta_temp.station_num);
                    strncpy(sta_temp.first,line,34);sta_temp.first[34]='\0';
                    if(n_status!=1)
                       {
                         //重新对错误数据进行异常值赋值，并提醒错误数据
                         sprintf(sta_temp.station_num,"99999");
                         printf("format.err-[基本信息字节数正确，格式错误]-line:%d-file:%s\n",line_num,filename);
                       }
                    num=num+2;
                 }
               else if(reglen(line)==135||line[0]=='N')//水文站末尾行为NNNN ,由于有空格，所以reglen(line)==5
                 {
                    //读取能见度或者NNNN数据
                    sprintf(sta_temp.four,line);
                    sprintf(sta_temp.filename,filename);
                    //将本次台站的数据写入
                    if(strcmp(sta_temp.station_num,"99999")!=0 && strcmp(sta_temp.o_time,"99999999999999")!=0)
                      {
                         //格式正确的数据写入
                         p=pTail;
                         pTail=(sta_data*)malloc(LEN_DATA);
                         strtodouble(pTail,&sta_temp);//将字符串格式转换为double格式
                         p->next=pTail;
                      }
                    //2、置空结构体
                    memset(&sta_temp,0,sizeof(struct station_temp));
                    num++;
                 }
               else
                 {
                    fprintf(log_fp,"%s %d 这个站的数据格式错误，以下数据不再读取\n",filename,line_num);
                    pTail->next=NULL;
                    return(pHead);
                 }
           }
      }
    fclose(reg_fp);
    pTail->next=NULL;
    return(pHead);
}

int getthresholdbymon(threshold *th,const char *mon)
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
                  strcpy(th->mon,data[index]);
               if(index==(nColumn+1))
                  th->wind_min=atof(data[index])/10;
               if(index==(nColumn+2))
                  th->wind_max=atof(data[index])/10;
               if(index==(nColumn+3))
                  th->rain_min=atof(data[index])/10;
               if(index==(nColumn+4))
                  th->rain_max=atof(data[index])/10;                 
               if(index==(nColumn+5))
                {
                  char temp_min[4],zf_flag_min[2];
                  substr(temp_min,data[index],1,3);
                  substr(zf_flag_min,data[index],0,1);
                  if(zf_flag_min[0]=='0')
                    th->temp_min=atof(temp_min)/10;
                  else
                    th->temp_min=atof(temp_min)/10*(-1);
                }
               if(index==(nColumn+6))
                {
                  char temp_max[4],zf_flag_max[2];
                  substr(temp_max,data[index],1,3);
                  substr(zf_flag_max,data[index],0,1);
                  if(zf_flag_max[0]=='0')
                    th->temp_max=atof(temp_max)/10;
                  else
                    th->temp_max=atof(temp_max)/10*(-1);
                }
               if(index==(nColumn+7))
                  th->pres_min=atof(data[index])/10;
               if(index==(nColumn+8))
                  th->pres_max=atof(data[index])/10;
               if(index==(nColumn+9))
                {
                  char g_temp_min[4],g_zf_flag_min[2];
                  substr(g_temp_min,data[index],1,3);
                  substr(g_zf_flag_min,data[index],0,1);
                  if(g_zf_flag_min[0]=='0')
                    th->g_temp_min=atof(g_temp_min)/10;
                  else
                    th->g_temp_min=atof(g_temp_min)/10*(-1);
                }
               if(index==(nColumn+10))
                {
                  char g_temp_max[4],g_zf_flag_max[2];
                  substr(g_temp_max,data[index],1,3);
                  substr(g_zf_flag_max,data[index],0,1);
                  if(g_zf_flag_max[0]=='0')
                    th->g_temp_max=atof(g_temp_max)/10;
                  else
                    th->g_temp_max=atof(g_temp_max)/10*(-1);
                } 
               if(index==(nColumn+11))
                  th->s_pres_min=atof(data[index])/10;
               if(index==(nColumn+12))
                  th->s_pres_max=atof(data[index])/10;
               index++;
            }
        }
    }
  sqlite3_free_table(data);
  sqlite3_close(db); //关闭数据库*/
  return 1;
}

//返回值为-1表示红色，返回值为0表示绿色，返回值为1表示黄色
int qc_node(sta_data *pNode,struct qc_result *result)
{
   //定义阈值数据结构
   struct qc_threshold th;
   
   //获取观测数据的观测时间
   char mon[3];
   substr(mon,pNode->o_time,4,2);//月份

   //根据月份mon获取阈值数据th
   getthresholdbymon(&th,mon);
   
   //以下根据阈值和观测值进行单节点质量控制
   //1.对两米风速进行质控
   if((pNode->wind2 > th.wind_max || pNode->wind2 < th.wind_min) && pNode->wind2 !=999.999)//质控不过
   {
      strncpy(result->station_num,pNode->station_num,5);result->station_num[5]='\0';
      strncpy(result->o_time,pNode->o_time,14);result->o_time[14]='\0';
      strncpy(result->ele,"wind2",15);result->ele[15]='\0';
      result->ele_value=pNode->wind2;
      strncpy(result->filename,pNode->filename,63);result->filename[63]='\0';
      return(-1);
   }
   else
   {
      //printf("对%s站的%s时次的资料进行质控：\n",pNode->station_num,pNode->o_time);
      //printf("两分钟风速最小值%f < 观测值%f < 两分钟风速最大值%f\n",th.wind_min,pNode->wind2,th.wind_max);
   }
   
   //2.对十米风速进行质控
   if((pNode->wind10 > th.wind_max || pNode->wind10 < th.wind_min) && pNode->wind10!=999.999)//质控不过
   {
      strncpy(result->station_num,pNode->station_num,5);result->station_num[5]='\0';
      strncpy(result->o_time,pNode->o_time,14);result->o_time[14]='\0';
      strncpy(result->ele,"wind10",15);result->ele[15]='\0';
      result->ele_value=pNode->wind10;
      strncpy(result->filename,pNode->filename,63);result->filename[63]='\0';
      return(-1);
   }
   else
   {
      //printf("对%s站的%s时次的资料进行质控：\n",pNode->station_num,pNode->o_time);
      //printf("十分钟风速最小值%f < 观测值%f < 十分钟风速最大值%f\n",th.wind_min,pNode->wind10,th.wind_max);
   }
   
   //3.对最大风速进行质控
   if((pNode->wind_max > th.wind_max || pNode->wind_max < th.wind_min) && pNode->wind_max!=999.999)//质控不过
   {
      strncpy(result->station_num,pNode->station_num,5);result->station_num[5]='\0';
      strncpy(result->o_time,pNode->o_time,14);result->o_time[14]='\0';
      strncpy(result->ele,"wind_max",15);result->ele[15]='\0';
      result->ele_value=pNode->wind_max;
      strncpy(result->filename,pNode->filename,63);result->filename[63]='\0';
      return(-1);
   }
   else
   {
      //printf("对%s站的%s时次的资料进行质控：\n",pNode->station_num,pNode->o_time);
      //printf("最大分钟风速最小值%f < 观测值%f < 最大风速最大值%f\n",th.wind_min,pNode->wind_max,th.wind_max);
   }
   
   //4.对瞬时风速进行质控
   if((pNode->wind_s > th.wind_max || pNode->wind_s < th.wind_min) && pNode->wind_s != 999.999)//质控不过
   {
      strncpy(result->station_num,pNode->station_num,5);result->station_num[5]='\0';
      strncpy(result->o_time,pNode->o_time,14);result->o_time[14]='\0';
      strncpy(result->ele,"wind_s",15);result->ele[15]='\0';
      result->ele_value=pNode->wind_s;
      strncpy(result->filename,pNode->filename,63);result->filename[63]='\0';
      return(-1);
   }
   else
   {
      //printf("对%s站的%s时次的资料进行质控：\n",pNode->station_num,pNode->o_time);
      //printf("瞬时风速最小值%f < 观测值%f < 瞬时风速最大值%f\n",th.wind_min,pNode->wind2,th.wind_max);
   }
 
   //5.对极大风速进行质控
   if((pNode->wind_j > th.wind_max || pNode->wind_j < th.wind_min) && pNode->wind_j != 999.999)//质控不过
   {
      strncpy(result->station_num,pNode->station_num,5);result->station_num[5]='\0';
      strncpy(result->o_time,pNode->o_time,14);result->o_time[14]='\0';
      strncpy(result->ele,"wind_j",15);result->ele[15]='\0';
      result->ele_value=pNode->wind_j;
      strncpy(result->filename,pNode->filename,63);result->filename[63]='\0';
      return(-1);
   }
   else
   {
      //printf("对%s站的%s时次的资料进行质控：\n",pNode->station_num,pNode->o_time);
      //printf("极大风速最小值%f < 观测值%f < 极大风速最大值%f\n",th.wind_min,pNode->wind2,th.wind_max);
   }
 
   //6.对小时降水量进行质控
   if((pNode->rain > th.rain_max || pNode->rain < th.rain_min) && pNode->rain != 999.999)//质控不过
   {
      strncpy(result->station_num,pNode->station_num,5);result->station_num[5]='\0';
      strncpy(result->o_time,pNode->o_time,14);result->o_time[14]='\0';
      strncpy(result->ele,"rain",15);result->ele[15]='\0';
      result->ele_value=pNode->rain;
      strncpy(result->filename,pNode->filename,63);result->filename[63]='\0';
      return(1);
   }
   else
   {
      //printf("对%s站的%s时次的资料进行质控：\n",pNode->station_num,pNode->o_time);
      //printf("降水量最小值%f < 观测值%f < 降水量最大值%f\n",th.rain_min,pNode->rain,th.rain_max);
   }
   
   //7.对气温/温度进行质控
   if((pNode->tem > th.temp_max || pNode->tem < th.temp_min) && pNode->tem != 999.999)//质控不过
   {
      strncpy(result->station_num,pNode->station_num,5);result->station_num[5]='\0';
      strncpy(result->o_time,pNode->o_time,14);result->o_time[14]='\0';
      strncpy(result->ele,"tem",15);result->ele[15]='\0';
      result->ele_value=pNode->tem;
      strncpy(result->filename,pNode->filename,63);result->filename[63]='\0';
      return(-1);
   }
   else
   {
      //printf("对%s站的%s时次的资料进行质控：\n",pNode->station_num,pNode->o_time);
      //printf("气温最小值%f < 观测值%f < 气温最大值%f\n",th.temp_min,pNode->tem,th.temp_max);
   }
 
   return(0);
}

sta_data *qc_list(sta_data *pHead,FILE *qc_fp)
{
    //记录当前时间
    time_t timer;//time_t就是long int 类型
    struct tm  *tm_struct;//存储时间的结构体
    timer = time(NULL);
    tm_struct = localtime(&timer);//获取了时间结构体
    char qc_time[20];
    sprintf(qc_time,"%d-%02d-%02d %02d:%02d:%02d", \
    tm_struct->tm_year+1900,tm_struct->tm_mon+1,tm_struct->tm_mday,tm_struct->tm_hour,tm_struct->tm_min,tm_struct->tm_sec);

    sta_data *pMove;
    sta_data *pMovePre;
    pMovePre = pHead;
    pMove = pHead->next;
    while (pMove != NULL)
     {
        //测试输出pMove
        struct qc_result result;
        int status=qc_node(pMove,&result);

        //对pMove节点进行质量检查
        if(status==0)//绿色节点
          {
             //此节点不删除，pMovePre和pMove均进行移位操作，对下一个节点进行质控
             //printf("质控结果staion:%s,time:%s,ele:%s,value:%f,filename:%s\n",result.station_num,result.o_time,result.ele,result.ele_value,result.filename);
             pMovePre = pMovePre->next;
             pMove = pMove->next;
          }
        else if(status==1)//黄色节点
          {
             //此节点不删除，pMovePre和pMove均进行移位操作
             pMovePre = pMovePre->next;
             pMove = pMove->next;

             //对黄色节点进行持久化操作,日志?消息队列？
             fprintf(qc_fp,"%s warn %s %s %s %.2f %s\n",qc_time,result.station_num,result.o_time,result.ele,result.ele_value,result.filename);
          }
        else if(status==-1)//红色节点
          {
             //此节点删除
             pMovePre->next = pMove->next;
             free(pMove);
             pMove = pMovePre->next;

             //对黄色节点进行持久化操作,日志?消息队列？
             fprintf(qc_fp,"%s err %s %s %s %.2f %s\n",qc_time,result.station_num,result.o_time,result.ele,result.ele_value,result.filename);
          }
    }
    return pHead;
}

int main()
{
    //定义收集目录
    char *recv_path="/home/acenter/src/rqc/reg";
    
    //定义质控后的发送目录
    char *send_path="/home/acenter/src/rqc/send";
    
    //定义区域站文件名模版
    char awscard[]="Z_SURF_C_B\?\?\?-REG_*_O_AWS_FTM.txt";
    
    //记录格式检查错误日志
    char *formatlogpath="/home/acenter/src/rqc/log/format.log";
    FILE *format_fp;
    if((format_fp=fopen(formatlogpath,"at"))==NULL)
      {
         printf("open format_log faild \n");
         return 0;
      }
    
    //记录质量控制错误日志
    char *qclogpath="/home/acenter/src/rqc/log/qc.log";
    FILE *qc_fp;
    if((qc_fp=fopen(qclogpath, "at"))==NULL)
     {
        printf("open qc_log faild \n");
        return 0;
     }
    
    //读取收集目录
    DIR *recv_dp=opendir(recv_path);
    //遍历目录内的所有文件
    struct dirent *recv_entry;
    while((recv_entry=readdir(recv_dp))!=NULL)
      {
         if(match(recv_entry->d_name,awscard))
             {
                /*解析这个文件到站级数据结构,
                将解析的格式错误台站信息保存到日志文件*/
                sta_data *datahead=readregfile(recv_path,recv_entry->d_name,format_fp);
 
                /*对站级的数据进行质量控制
                对于质量不合格的写入日志记录*/
                if(datahead->next!=NULL)
                 {
                    qc_list(datahead,qc_fp);

                    //对质量合格的台站数据写入到发送目录
               
                    //1.定义文件名
                    char sendfile[1024];
                    sprintf(sendfile,"%s/%s",send_path,recv_entry->d_name);
                    FILE *file_fp;
                    if((file_fp=fopen(sendfile, "at"))==NULL)
                     {
                       printf("open file err\n");
                       return(0);
                     }
               
                    //将sta_data数据结构的first、second、three、four写入到质控后文件
                    sta_data *p=datahead;
                    p=p->next;
                    while(p!=NULL)
                     {
                        fprintf(file_fp,"%s\n%s\n%s\n",p->first,p->second,p->three);
                        if(p->four[0] != 'N')//写入能见度等少数站才有的观测数据
                           fprintf(file_fp,"%s\n",p->four);
                        p=p->next;
                     }
                    fprintf(file_fp,"NNNN");
                    fclose(file_fp);
                  }
                
                //释放内存
                sta_data *pointer;
                while(datahead!=NULL)
                 {
                    pointer = datahead;
                    datahead = datahead->next;
                    free(pointer);
                    pointer=NULL;
                 }
             }
      }
    fclose(format_fp);
    fclose(qc_fp);
    closedir(recv_dp);
    return 0;
}
