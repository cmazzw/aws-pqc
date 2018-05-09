#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
        else if(s[i]=='/' || s[i]=='-' || s[i]==' ')
          num++;
        else if(s[i]>='0'&&s[i]<='9')
          num++;
        i++;
      }
    return num;
}

//if((ch>='a') && (ch<='z'))  
//      printf("%c is a lowercase letter\n" ,ch);  
//else if ((ch>='A') && (ch<='Z'))  
//      print(" %c is a uppercase letter\n" ,ch);  
//else  
//      printf(" %c is not an alphabet letter\n" ,ch);  

void exchange(char *p1,char *p2)  
{  
    char temp[48];  
    strcpy(temp,p1);
    strcpy(p1,p2);
    strcpy(p2,temp);  
} 


int main()
{

   //char test[5]="////";
   //double s=atof(test);
   //printf("%f\n",s);
   char* src="abcd";
   char dest[23];
   strncpy(dest,src,22);
   dest[22]='\0';
   printf("%s\n",dest);
   printf("%d\n",strlen(dest));

/*
   char p[48]="zhangzhaowei";
   char d[48]="liqianni";
 
   printf("交换前:p%s\n",p);

   exchange(p,d);

   printf("交换后:p%s\n",p);
  
   //strcpy(d,p);
   //printf("strcpy result is %s\n",d);

   //d=p;
   //cpy(d,p);
   //printf("point result is %s\n",d);

   char line[300];
   sprintf(line,"Z_SURF_C_BEHB-REG_20180417170346_O_AWS_FTM.txt");
   //printf("first %s\n",line);
   //sprintf(line,"zhangzhaowei\n");
   //printf("second %s\n",line);
   printf("sizeof is %d\n",sizeof(line));
   printf("strlen is %d\n",strlen(line));
   printf("reglen is %d\n",reglen(line));*/
   return(1);
}
