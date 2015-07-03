#include "stdio.h"
#include "smart.h"
#include <string.h> 
#include <ctype.h>
#include "hdctl.h"


/*
	�Ͳ���ϵͳ��ص�,����ioctl��
	��дӲ�̲����ĺ�������smartcommandhandler����
*/
int ata_command_interface(int device, smart_command_set command, int select, char *data)
{
  unsigned char buff[STRANGE_BUFFER_LENGTH];
  // positive: bytes to write to caller.  negative: bytes to READ from
  // caller. zero: non-data command
  int copydata=0;
	
  const int HDIO_DRIVE_CMD_OFFSET = 4;

  // See struct hd_drive_cmd_hdr in hdreg.h.  Before calling ioctl()
  // buff[0]: ATA COMMAND CODE REGISTER
  // buff[1]: ATA SECTOR NUMBER REGISTER == LBA LOW REGISTER
  // buff[2]: ATA FEATURES REGISTER
  // buff[3]: ATA SECTOR COUNT REGISTER

  // Note that on return:
  // buff[2] contains the ATA SECTOR COUNT REGISTER
  
  // clear out buff.  Large enough for HDIO_DRIVE_CMD (4+512 bytes)
  memset(buff, 0, STRANGE_BUFFER_LENGTH);

  buff[0]=ATA_SMART_CMD;
  switch (command){
  case CHECK_POWER_MODE:
    buff[0]=ATA_CHECK_POWER_MODE;
    copydata=1;
    break;
  case READ_VALUES:
    buff[2]=ATA_SMART_READ_VALUES;
    buff[3]=1;
    copydata=512;
    break;
  case READ_THRESHOLDS:
    buff[2]=ATA_SMART_READ_THRESHOLDS;
    buff[1]=buff[3]=1;
    copydata=512;
    break;
  case READ_LOG:
    buff[2]=ATA_SMART_READ_LOG_SECTOR;
    buff[1]=select;
    buff[3]=1;
    copydata=512;
    break;
  case WRITE_LOG:
    break;
  case IDENTIFY:
    buff[0]=ATA_IDENTIFY_DEVICE;
    buff[3]=1;
    copydata=512;
    break;
  case PIDENTIFY:
    buff[0]=ATA_IDENTIFY_PACKET_DEVICE;
    buff[3]=1;
    copydata=512;
    break;
  case ENABLE:
    buff[2]=ATA_SMART_ENABLE;
    buff[1]=1;
    break;
  case DISABLE:
    buff[2]=ATA_SMART_DISABLE;
    buff[1]=1;
    break;
  case STATUS:
    // this command only says if SMART is working.  It could be
    // replaced with STATUS_CHECK below.
    buff[2]=ATA_SMART_STATUS;
    break;
  case AUTO_OFFLINE:
    buff[2]=ATA_SMART_AUTO_OFFLINE;
    buff[3]=select;   // YET NOTE - THIS IS A NON-DATA COMMAND!!
    break;
  case AUTOSAVE:
    buff[2]=ATA_SMART_AUTOSAVE;
    buff[3]=select;   // YET NOTE - THIS IS A NON-DATA COMMAND!!
    break;
  case IMMEDIATE_OFFLINE:
    buff[2]=ATA_SMART_IMMEDIATE_OFFLINE;
    buff[1]=select;
    break;
  case STATUS_CHECK:
    // This command uses HDIO_DRIVE_TASK and has different syntax than
    // the other commands.
    buff[1]=ATA_SMART_STATUS;
    break;
  default:
    printf("Unrecognized command %d in linux_ata_command_interface()\n");
        
    errno=ENOSYS;
    return -1;
  }
  
  // This command uses the HDIO_DRIVE_TASKFILE ioctl(). This is the
  // only ioctl() that can be used to WRITE data to the disk.
  if (command==WRITE_LOG) {    
    unsigned char task[sizeof(ide_task_request_t)+512];
    ide_task_request_t *reqtask=(ide_task_request_t *) task;
    task_struct_t      *taskfile=(task_struct_t *) reqtask->io_ports;
    int retval;

    memset(task,      0, sizeof(task));
    
    taskfile->data           = 0;
    taskfile->feature        = ATA_SMART_WRITE_LOG_SECTOR;
    taskfile->sector_count   = 1;
    taskfile->sector_number  = select;
    taskfile->low_cylinder   = 0x4f;
    taskfile->high_cylinder  = 0xc2;
    taskfile->device_head    = 0;
    taskfile->command        = ATA_SMART_CMD;
    
    reqtask->data_phase      = TASKFILE_OUT;
    reqtask->req_cmd         = IDE_DRIVE_TASK_OUT;
    reqtask->out_size        = 512;
    reqtask->in_size         = 0;
    
    // copy user data into the task request structure
    memcpy(task+sizeof(ide_task_request_t), data, 512);
      
    if ((retval=ioctl(device, HDIO_DRIVE_TASKFILE, task))) {
      if (retval==-EINVAL)
	printf("Kernel lacks HDIO_DRIVE_TASKFILE support; compile kernel with CONFIG_IDE_TASKFILE_IO set\n");
      return -1;
    }
    return 0;
  }
    
  // There are two different types of ioctls().  The HDIO_DRIVE_TASK
  // one is this:
  if (command==STATUS_CHECK){
    int retval;

    // NOT DOCUMENTED in /usr/src/linux/include/linux/hdreg.h. You
    // have to read the IDE driver source code.  Sigh.
    // buff[0]: ATA COMMAND CODE REGISTER
    // buff[1]: ATA FEATURES REGISTER
    // buff[2]: ATA SECTOR_COUNT
    // buff[3]: ATA SECTOR NUMBER
    // buff[4]: ATA CYL LO REGISTER
    // buff[5]: ATA CYL HI REGISTER
    // buff[6]: ATA DEVICE HEAD

    unsigned const char normal_lo=0x4f, normal_hi=0xc2;
    unsigned const char failed_lo=0xf4, failed_hi=0x2c;
    buff[4]=normal_lo;
    buff[5]=normal_hi;
    
    if ((retval=ioctl(device, HDIO_DRIVE_TASK, buff))) {
      if (retval==-EINVAL) {
	printf("Error SMART Status command via HDIO_DRIVE_TASK failed");
	printf("Rebuild older linux 2.2 kernels with HDIO_DRIVE_TASK support added\n");
      }
      else
	printf("Error SMART Status command failed");
      return -1;
    }
    
    // Cyl low and Cyl high unchanged means "Good SMART status"
    if (buff[4]==normal_lo && buff[5]==normal_hi)
      return 0;
    
    // These values mean "Bad SMART status"
    if (buff[4]==failed_lo && buff[5]==failed_hi)
      return 1;
    
    // We haven't gotten output that makes sense; print out some debugging info
    printf("Error SMART Status command failed");
    printf("Register values returned from SMART Status command are:\n");
    printf("CMD=0x%02x\n",(int)buff[0]);
    printf("FR =0x%02x\n",(int)buff[1]);
    printf("NS =0x%02x\n",(int)buff[2]);
    printf("SC =0x%02x\n",(int)buff[3]);
    printf("CL =0x%02x\n",(int)buff[4]);
    printf("CH =0x%02x\n",(int)buff[5]);
    printf("SEL=0x%02x\n",(int)buff[6]);
    return -1;   
  }
  
#if 1
  // Note to people doing ports to other OSes -- don't worry about
  // this block -- you can safely ignore it.  I have put it here
  // because under linux when you do IDENTIFY DEVICE to a packet
  // device, it generates an ugly kernel syslog error message.  This
  // is harmless but frightens users.  So this block detects packet
  // devices and make IDENTIFY DEVICE fail "nicely" without a syslog
  // error message.
  //
  // If you read only the ATA specs, it appears as if a packet device
  // *might* respond to the IDENTIFY DEVICE command.  This is
  // misleading - it's because around the time that SFF-8020 was
  // incorporated into the ATA-3/4 standard, the ATA authors were
  // sloppy. See SFF-8020 and you will see that ATAPI devices have
  // *always* had IDENTIFY PACKET DEVICE as a mandatory part of their
  // command set, and return 'Command Aborted' to IDENTIFY DEVICE.
  if (command==IDENTIFY || command==PIDENTIFY){
    unsigned short deviceid[256];
    // check the device identity, as seen when the system was booted
    // or the device was FIRST registered.  This will not be current
    // if the user has subsequently changed some of the parameters. If
    // device is a packet device, swap the command interpretations.
    if (!ioctl(device, HDIO_GET_IDENTITY, deviceid) && (deviceid[0] & 0x8000))
      buff[0]=(command==IDENTIFY)?ATA_IDENTIFY_PACKET_DEVICE:ATA_IDENTIFY_DEVICE;
  }
#endif
  
  // We are now doing the HDIO_DRIVE_CMD type ioctl.
  if ((ioctl(device, HDIO_DRIVE_CMD, buff)))
    return -1;

  // CHECK POWER MODE command returns information in the Sector Count
  // register (buff[3]).  Copy to return data buffer.
  if (command==CHECK_POWER_MODE)
    buff[HDIO_DRIVE_CMD_OFFSET]=buff[2];

  // if the command returns data then copy it back
  if (copydata)
    memcpy(data, buff+HDIO_DRIVE_CMD_OFFSET, copydata);

  return 0; 
}


/*
	��smart�������ж�д�����ĺ�������������ĺ���������������smart����
	device	:	��Ӳ�̽ڵ�õ���fd
	command	:	�����������
	select	:	����ѡ��
	data	:	�������ݽ����Ļ�����ָ��
*/
int smartcommandhandler(int device, smart_command_set command, int select, char *data)
{
  int retval;

  // This conditional is true for commands that return data
  int getsdata=(command==PIDENTIFY || 
                command==IDENTIFY || 
                command==READ_LOG || 
                command==READ_THRESHOLDS || 
                command==READ_VALUES ||
		command==CHECK_POWER_MODE);

  int sendsdata=(command==WRITE_LOG);
  
  if ((getsdata || sendsdata) && !data){
   printf("REPORT-IOCTL: Unable to execute command  : data destination address is NULL\n" );
    return -1;
  }
  
  // The reporting is cleaner, and we will find coding bugs faster, if
  // the commands that failed clearly return empty (zeroed) data
  // structures
  if (getsdata) {
    if (command==CHECK_POWER_MODE)
      data[0]=0;
    else
      memset(data, '\0', 512);
  }

  // In case the command produces an error, we'll want to know what it is:
  errno=0;
  // now execute the command
  return ata_command_interface(device, command, select, data);
}


//���豸�Ļ�����Ϣ����drive�ṹ��.�����ظ�ֵ���ɹ�����0
int ataReadHDIdentity (int devicefd, struct ata_identify_device *drive)
{
  unsigned short *rawshort=(unsigned short *)drive;
  unsigned char  *rawbyte =(unsigned char  *)drive;

  // See if device responds either to IDENTIFY DEVICE or IDENTIFY
  // PACKET DEVICE
  if ((smartcommandhandler(devicefd, IDENTIFY, 0, (char *)drive))){
    if (smartcommandhandler(devicefd, PIDENTIFY, 0, (char *)drive)){
      return -1; 
    }
  }

  // If this is a PACKET DEVICE, return device type
  if (rawbyte[1] & 0x80)
    return -EPERM;
  
  // Not a PACKET DEVICE
  return 0;
}

// Copies n bytes (or n-1 if n is odd) from in to out, but swaps adjacents
// bytes.
void swapbytes(char *out, const char *in, size_t n)
{
  size_t i;

  for (i = 0; i < n; i += 2) {
    out[i]   = in[i+1];
    out[i+1] = in[i];
  }
}


// Copies in to out, but removes leading and trailing whitespace.
void trim(char *out, const char *in)
{
  int i, first, last;

  // Find the first non-space character (maybe none).
  first = -1;
  for (i = 0; in[i]; i++)
    if (!isspace((int)in[i])) {
      first = i;
      break;
    }

  if (first == -1) {
    // There are no non-space characters.
    out[0] = '\0';
    return;
  }

  // Find the last non-space character.
  for (i = strlen(in)-1; i >= first && isspace((int)in[i]); i--)
    ;
  last = i;

  strncpy(out, in+first, last-first+1);
  out[last-first+1] = '\0';
}


// Convenience function for formatting strings from ata_identify_device
void formatdriveidstring(char *out, const char *in, int n)
{
  char tmp[65];

  n = n > 64 ? 64 : n;
  swapbytes(tmp, in, n);
  tmp[n] = '\0';
  trim(out, tmp);
}






// ����ataӲ�̵Ĵ�����������ֵ��ʾ����
int ataReadErrorLog (int devicefd)
{      
  struct ata_smart_errorlog data;
  
  // get data from device
  if (smartcommandhandler(devicefd, READ_LOG, 0x01, (char *)(&data)))
  {
    printf("Error SMART Error Log Read failed");
    return -1;
  }

  return data.ata_error_count;
}


//  ��ȡSMART ����,�ҵ�ָ����ŵ����Ե�ָ��ֵ�����ء�ʧ�ܷ��ظ�ֵ
//	devicefd: ���豸�ڵ��������
//  attribute_no: ���Եı�ţ���smart.h���ж���
//  valuetype:	  ֵ�����ͣ���:��ǰ,���..����smarg.h���ж��壩	
long long ataReadSmartValues(int devicefd, int attribute_no, int valuetype)
{      
  struct ata_smart_values data;
  int i;
    
  if (smartcommandhandler(devicefd, READ_VALUES, 0, (char *)&data))
  {
    printf("Error SMART Values Read failed");
    return -1;
  }
  

  // step through all vendor attributes
  for (i=0; i<NUMBER_ATA_SMART_ATTRIBUTES; i++)
  {
	    struct ata_smart_attribute *disk=data.vendor_attributes+i;
	    
	    // consider only valid attributes (allowing some screw-ups in the
	    // thresholds page data to slip by)
	    if (disk->id == attribute_no)
	    {
		  	long long rawvalue=0;
	
	      	int j;
	      	switch(valuetype)
	      	{
	      		case(RAWVALUE):
							  // convert the six individual bytes to a long long (8 byte) integer.
							  // This is the value that we'll eventually return.
							 
							  // This looks a bit roundabout, but is necessary.  Don't
							  // succumb to the temptation to use raw[j]<<(8*j) since under
							  // the normal rules this will be promoted to the native type.
							  // On a 32 bit machine this might then overflow.	
							  for (j=0; j<6; j++) 
							  {
							    long long temp;
							    temp = disk->raw[j];
							    temp <<= 8*j;
							    rawvalue |= temp;
							  }	  
						  	
						  	
						  	
						  	// Return the full value
						 		return rawvalue;
						 		
				case(WORSTVALUE):	return disk->worst;
				default:		return -EINVAL;
								
			}		
			
		}
	}
    return -ENOENT; //û���ҵ�
}

// Returns nonzero if region of memory contains non-zero entries
int nonempty(unsigned char *testarea,int n){
  int i;
  for (i=0;i<n;i++)
    if (testarea[i])
      return 1;
  return 0;
}

//���س��̱��,���ڴ���һЩ�泧�̲�ͬ����ͬ�����
//ʧ�ܷ��ظ�ֵ
int get_hd_manufactor(int devicefd)
{
	struct ata_identify_device  drive;
	
	char model[40];
	if(ataReadHDIdentity(devicefd, &drive)== 0)
	{
		formatdriveidstring(model, (char *)drive.model,40);
		if(strncmp(model,"Maxtor",strlen("Maxtor"))==0)
			return MAXTOR;
		if(strncmp(model,"ST",strlen("ST"))==0)
			return SEAGATE;
		if(strncmp(model,"WD",strlen("WD"))==0)
			return WD;
		if(strncmp(model,"Hitachi",strlen("Hitachi"))==0)
			return HITACHI;
	}
	
	return -ENODEV;
		
}


//����smart���ԣ�testtype: 0-�̣�1-��
//����ֵ,0��ʾ�ɹ�,������ʾʧ��
int ataSmartTest(int devicefd, int testtype) 
{     
  int ret;
  // Now send the command to test
  ret=smartcommandhandler(devicefd, IMMEDIATE_OFFLINE, testtype+1, NULL);
  return ret;
}





//�������һ��ָ�����͵��Լ�Ľ����0��ʾͨ����1��ʾʧ�ܣ�2��ʾ������,3��ʾ������
//���ظ�ֵ��ʾ������
//testtype:	0-�̲��ԣ�1-������
//percent_done:����ڽ�����,��ɵİٷֱ�

int ataGetSelfTestLog (int devicefd, int testtype ,int *percent_done )
{

	
	struct ata_smart_selftestlog data;
	int testno=0,retval =0;
	long stat = -1;
	int i,j;
	struct ata_smart_selftestlog_struct *log;
	
	if(percent_done == NULL)
		return -EINVAL;
  	// get data from device
  	if (smartcommandhandler(devicefd, READ_LOG, 0x06, (char *)&data))
  	{
    	printf("Error SMART Error Self-Test Log Read failed");
    	return -1;
  	}

	 // print log      
	 
	 for(i=0; i<=20; i++) //�ȼ����û�����ڽ��е�
	 {
	 	log = data.selftest_struct+((i+data.mostrecenttest)%21);
	 	if ((nonempty((unsigned char*)log,sizeof(*log)))&&(log->selftestnumber == testtype+1)&&(((log->selfteststatus)>>4)==15))
	 	{
	 		 stat = TEST_RUNNING;
	 		 *percent_done =100-10*((log->selfteststatus)&0xf);
	 		 break;
		}	     		
	 }
	 if(stat == TEST_RUNNING)  //Ŀǰ�н����еĸ����Ͳ���
	 	return stat;
	 	
	 	
	 //û���ҵ������е�	
 	 for (i=20;i>=0;i--)
 	{    
	   

	    // log is a circular buffer
	    j=(i+data.mostrecenttest)%21;
	    log=data.selftest_struct+j;
		
	    if (nonempty((unsigned char*)log,sizeof(*log)))
	    {
	      if  (log->selftestnumber == testtype+1)  //1:short offline 2:extended offline
	      {
		      // test status
		      	switch((log->selfteststatus)>>4)
		      	{
			      case  0:stat = TEST_PASSED;	break;//Completed without error
			      
			      case  3: 					//Fatal or unknown error 
			      case  4: 					//Completed: unknown failure
			      case  5: 					//Completed: electrical failure
			      case  6:					//Completed: servo/seek failure
			      case  7:					//Completed: read failure 
			      case  8:stat =TEST_FAILED; break;	//Completed: handling damage??
				
				  case 15:stat =TEST_RUNNING;	//self-test routine in progress
				  		  *percent_done =100-10*((log->selfteststatus)&0xf);
			     		  break; 
			      case  1:				  	//Aborted by host 
			      case  2:					//Interrupted (host reset) 
			      default:stat = TEST_UNAVILABLE; break;	//Unknown/reserved test status 
	     		 }
	     		 break;
	     		 
			}	
	      }

    }
     return stat;

  }
 








/*
	ͨ��lbaѰַ������̵�������С��������
	������0��ֵ
*/
long long determine_capacity(struct ata_identify_device *drive)
{

  unsigned short command_set_2  = drive->command_set_2;
  unsigned short capabilities_0 = drive->words047_079[49-47];
  unsigned short sects_16       = drive->words047_079[60-47];
  unsigned short sects_32       = drive->words047_079[61-47];
  unsigned short lba_16         = drive->words088_255[100-88];
  unsigned short lba_32         = drive->words088_255[101-88];
  unsigned short lba_48         = drive->words088_255[102-88];
  unsigned short lba_64         = drive->words088_255[103-88];
  long long capacity_short=0, capacity=0, threedigits, power_of_ten;
  int started=0,k=1000000000;


  // if drive supports LBA addressing, determine 32-bit LBA capacity
  if (capabilities_0 & 0x0200) {
    capacity_short = (unsigned int)sects_32 << 16 | 
                     (unsigned int)sects_16 << 0  ;
    
    // if drive supports 48-bit addressing, determine THAT capacity
    if ((command_set_2 & 0xc000) == 0x4000 && (command_set_2 & 0x0400))
      capacity = (uint64_t)lba_64 << 48 | 
	         (uint64_t)lba_48 << 32 |
	         (uint64_t)lba_32 << 16 | 
	         (uint64_t)lba_16 << 0  ;
    
    // choose the larger of the two possible capacities
    if (capacity_short>capacity)
      capacity=capacity_short;
  }

  // turn sectors into bytes
  capacity_short = (capacity *= 512);

  return capacity/1000;
}


