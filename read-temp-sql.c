#define _FILE_OFFSET_BITS 64
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
//#include <microhttpd.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <mysql.h>
#include <my_global.h>
#include <signal.h>
#include <pthread.h>
#include <getopt.h>
#include "read-temp-sql.h"

struct user_options userOpts;
MYSQL *conn;

int Daemon(void);

float getTempFromSensor(int sensor){
	int d;
	switch (sensor){
	    case 1: d = open("/sys/bus/w1/devices/28-000008e64cc0/w1_slave", O_RDONLY);break;
	    case 2: d = open("/sys/bus/w1/devices/28-000008e871cd/w1_slave", O_RDONLY);break;
	    case 3: d = open("/sys/bus/w1/devices/28-000008e4f9fa/w1_slave", O_RDONLY);break;
	}
	char dataT1[512];
	char *token;
	read(d, dataT1, 512);
	token = strtok (dataT1,"t\=");
	while (token != NULL){
		if(strtok (NULL, "t\=") != NULL){
			token = strtok (NULL, "t\=");
		}else{
			break;
		}
	}
	close (d);
	return  (float)atoi(token)/1000;
}

void sigsegv_signal (int signum){
	struct tm *u;
  	char s1[40] ;
  	time_t currTime;
	FILE* out_file;
	out_file = fopen ( "/var/log/temp_log.txt","a");
	currTime = time(NULL);
	u = localtime (&currTime);
	strftime(s1, 80, "%d.%m.%Y %H:%M:%S ", u);
	fprintf (out_file,"%s Procees halted by SIGSEGV  PID %d \n",s1, getpid());
	fclose (out_file);
	mysql_close(conn);
	signal (signum, SIG_DFL);
        exit (3);
}

void sigint_signal (int signum){
	struct tm *u;
  	char s1[40] ;
  	time_t currTime;
	FILE* out_file;
	out_file = fopen ( "/var/log/temp_log.txt","a");
	currTime = time(NULL);
	u = localtime (&currTime);
	strftime(s1, 80, "%d.%m.%Y %H:%M:%S ", u);
	fprintf (out_file,"%s Procees halted by ctrl+C (SIGINT)  PID %d \n",s1, getpid());
	fclose (out_file);
	mysql_close(conn);
	signal (signum, SIG_DFL);
        exit (3);
}

void sigterm_signal (int signum){
	struct tm *u;
  	char s1[40] ;
  	time_t currTime;
	FILE* out_file;
	out_file = fopen ( "/var/log/temp_log.txt","a");
	currTime = time(NULL);
	u = localtime (&currTime);
	strftime(s1, 80, "%d.%m.%Y %H:%M:%S ", u);
	fprintf (out_file,"%s Procees halted by SIGTERM  PID %d \n",s1, getpid());
	fclose (out_file);
	mysql_close(conn);
	signal (signum, SIG_DFL);
        exit (3);
}


int Daemon(void) {

	
	int res,i=10;
	struct tm *u;
  	char s1[80] ;
  	time_t currTime;
	FILE* out_file;
	char se1[10],se2[10],se3[10],se_out[200];
	
	out_file = fopen ( "/var/log/temp_log.txt","a");
        if(!out_file) {
                    perror("File opening failed");
                    return EXIT_FAILURE;
        }
	currTime = time(NULL);
	u = localtime (&currTime);
	strftime(s1, 80, "%d.%m.%Y %H:%M:%S ", u);
	fprintf (out_file,"%s Procees started in daemon  PID %d \n",s1, getpid());
	
	
	conn = mysql_init(NULL);
  		if(conn == NULL)
  		{
    		    // Если дескриптор не получен - выводим сообщение об ошибке
    		    currTime = time(NULL);
		    u = localtime (&currTime);
		    strftime(s1, 80, "%d.%m.%Y %H:%M:%S ", u);
		    fprintf(out_file, "%s Error: can't create MySQL-descriptor\n",s1);
    		    fclose (out_file);
		    exit(1);
  		}
		else {
		    currTime = time(NULL);
		    u = localtime (&currTime);
		    strftime(s1, 80, "%d.%m.%Y %H:%M:%S ", u);
		    fprintf(out_file, "%s Created MySQL-descriptor  %d\n",s1,conn);
    		    
		}
	// Подключаемся к серверу
  	if(!mysql_real_connect(conn,
		 "sip.mtm-pro.ru",
                    "mtmproru_kyrex",
                   "26pkd80fu",
                    "mtmproru_kyrex",
                    7080,
                    NULL,
                    0
                    ))
  	        {
    	// Если нет возможности установить соединение с сервером 
    	// базы данных выводим сообщение об ошибке
    		fprintf(out_file,"%s Error: can't connect to database %s\n",s1,mysql_error(conn));
		}
  		else
  		{
    		    // Если соединение успешно установлено выводим фразу - "Success!"
    		    currTime = time(NULL);
		    u = localtime (&currTime);
		    strftime(s1, 80, "%d.%m.%Y %H:%M:%S ", u);
		    fprintf(out_file, "%s  Connection established \n",s1);
  		} 

	fclose (out_file);
	while(1){
		out_file = fopen ( "/var/log/temp_log.txt","a");
                if(!out_file) {
                    perror("File opening failed");
                    //return EXIT_FAILURE;
    		}
		
		currTime = time(NULL);
		u = localtime (&currTime);
		strftime(s1, 80, "%d.%m.%Y %H:%M:%S ", u);
		
  		
  		// Устанавливаем кодировку соединения, чтобы предотвратить
		// искажения русского текста
		if(mysql_query(conn, "SET NAMES 'utf8'") != 0)
		{
		    // Если кодировку установить невозможно - выводим 
		    // сообщение об ошибке
		    fprintf(out_file, "%s Error: can't set character set  %s\n",s1,mysql_error(conn));
		    
		}
		sprintf ( se1, "%5.2f", getTempFromSensor(1) );
		sprintf ( se2, "%5.2f", getTempFromSensor(2) );
		//sprintf ( se3, "%5.2f", getTempFromSensor(3) );
		
		sprintf(se_out,"INSERT INTO `temp_village` (`Date`, `Sens1`, `Sens2`, `Sens3`, `Sens4`) VALUES (TIMESTAMP(CURRENT_TIMESTAMP), '%s', '%s', '%s', '%s')",se1,se2,se2,se2);
		
		if(mysql_query(conn, se_out ) != 0)
		{
		    // Если добавить запись не получилось - выводим 
		    // сообщение об ошибке
		    fprintf(out_file, "%s Error: can't execute INSERT-query  %s\n",s1,mysql_error(conn));
		    if(!mysql_real_connect(conn,"sip.mtm-pro.ru","mtmproru_kyrex", "26pkd80fu", "mtmproru_kyrex", 7080, NULL, 0 )) {
	        	// Если нет возможности установить соединение с сервером 
    			// базы данных выводим сообщение об ошибке
    			fprintf(out_file,"%s Error: can't connect to database %s\n",s1,mysql_error(conn));
		    }
  		    else
  		    {
    			// Если соединение успешно установлено
    			currTime = time(NULL);
			u = localtime (&currTime);
		        strftime(s1, 80, "%d.%m.%Y %H:%M:%S ", u);
			fprintf(out_file, "%s  Connection established \n",s1);
  		    } 
		}
		
		fclose (out_file);
		sleep(userOpts.NUM_SECONDS);
		
		
		
	}
	
	mysql_close(conn);
	return 0;

}


int main(int argc, char **argv) {
	
	if(argc == 1) { // если запускаем без аргументов, выводим справку
        printf("Use options -time XX or -stop");
        return 0;
	}
	char *opts = "t:d"; // доступные опции, каждая принимает аргумент
	int opt; // каждая следующая опция попадает сюда
    
	while((opt = getopt(argc, argv, opts)) != -1) { // вызываем getopt пока она не вернет -1
    	    switch(opt) {
        	case 't': // если опция -a, преобразуем строку с аргументом в число
            	    userOpts.NUM_SECONDS = atoi(optarg);
            	    printf ("Time is set to %s\n", optarg);
		    
		    break;
                case 'd': // тоже для -b
                    printf ("Try to stop daemon \n", optarg);
                    
		    break;
                
    	    }
	}

	
	pid_t parpid, sid;
	char se2[100],se1[100],se3[100];
	//userOpts.NUM_SECONDS = 20;
	userOpts.sensor1Enabled = 1;
	userOpts.sensor2Enabled = 1;
	userOpts.temperatureAlarmMax = 17;
	userOpts.temperatureAlarmMin = -5;
	userOpts.notifyPerioidMinutes = 5;
	signal(SIGSEGV, sigsegv_signal);
	signal(SIGINT, sigint_signal);
	signal(SIGTERM, sigterm_signal);
	
  	parpid = fork(); //создаем дочерний процесс
   	
	if(parpid < 0) {
        	exit(1);
    	} 
	else if(parpid != 0) {
        	exit(0);
    	} 
    	umask(0);//даем права на работу с фс
    	sid = setsid();//генерируем уникальный индекс процесса
    	if(sid < 0) {
        	exit(1);
    	}
    	if((chdir("/")) < 0) {//выходим в корень фс
        	exit(1);
    	}
    	
	close(STDIN_FILENO);//закрываем доступ к стандартным потокам ввода-вывода
    	close(STDOUT_FILENO);
    	close(STDERR_FILENO);
    	
    	return Daemon();
	
	
}
