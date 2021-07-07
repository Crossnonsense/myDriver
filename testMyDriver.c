#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<fcntl.h>
#include<string.h>
#include<unistd.h>



int main(){
   int ret, fd;
   char buffer[10] = "";
   unsigned int u_int;
   printf("Starting device test code example...\n");
   fd = open("/dev/myDriver", O_RDWR);             
   if (fd < 0){
      perror("Failed to open the device...");
      return errno;
   }
   
   int i = 0;
   while (i < 3)
   {
   	i++;
   	printf("Reading from the device...\n");
   	ret = read(fd, buffer, sizeof(buffer));
   	u_int = atoi(buffer);        
   	if (ret < 0){
      		perror("Failed to read the message from the device.");
      		return errno;
   	}
   	printf("The received number is: [%u]\n", u_int);
   }
   
   printf("End of the program\n");
   return 0;
}
