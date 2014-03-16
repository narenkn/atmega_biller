//********************************************************
// **** ROUTINES FOR FAT32 IMPLEMATATION OF SD CARD *****
//********************************************************
//Controller		: ATmega32 (Clock: 8 Mhz-internal)
//Compiler			: AVR-GCC (winAVR with AVRStudio-4)
//Project Version	: DL_1.0
//Author			: CC Dharmani, Chennai (India)
//			  		  www.dharmanitech.com
//Date				: 10 May 2011
//*********************************************************

#ifndef FAT32_H
#define FAT32_H

//Structure to access Master Boot Record for getting info about partioions
struct MBRinfo_Structure{
uint8_t	nothing[446];		//ignore, placed here to fill the gap in the structure
uint8_t	partitionData[64];	//partition records (16x4)
unsigned int	signature;		//0xaa55
};

//Structure to access info of the first partioion of the disk 
struct partitionInfo_Structure{ 				
uint8_t	status;				//0x80 - active partition
uint8_t 	headStart;			//starting head
unsigned int	cylSectStart;		//starting cylinder and sector
uint8_t	type;				//partition type 
uint8_t	headEnd;			//ending head of the partition
unsigned int	cylSectEnd;			//ending cylinder and sector
unsigned long	firstSector;		//total sectors between MBR & the first sector of the partition
unsigned long	sectorsTotal;		//size of this partition in sectors
};

//Structure to access boot sector data
struct BS_Structure{
uint8_t jumpBoot[3]; //default: 0x009000EB
uint8_t OEMName[8];
unsigned int bytesPerSector; //deafault: 512
uint8_t sectorPerCluster;
unsigned int reservedSectorCount;
uint8_t numberofFATs;
unsigned int rootEntryCount;
unsigned int totalSectors_F16; //must be 0 for FAT32
uint8_t mediaType;
unsigned int FATsize_F16; //must be 0 for FAT32
unsigned int sectorsPerTrack;
unsigned int numberofHeads;
unsigned long hiddenSectors;
unsigned long totalSectors_F32;
unsigned long FATsize_F32; //count of sectors occupied by one FAT
unsigned int extFlags;
unsigned int FSversion; //0x0000 (defines version 0.0)
unsigned long rootCluster; //first cluster of root directory (=2)
unsigned int FSinfo; //sector number of FSinfo structure (=1)
unsigned int BackupBootSector;
uint8_t reserved[12];
uint8_t driveNumber;
uint8_t reserved1;
uint8_t bootSignature;
unsigned long volumeID;
uint8_t volumeLabel[11]; //"NO NAME "
uint8_t fileSystemType[8]; //"FAT32"
uint8_t bootData[420];
unsigned int bootEndSignature; //0xaa55
};


//Structure to access FSinfo sector data
struct FSInfo_Structure
{
unsigned long leadSignature; //0x41615252
uint8_t reserved1[480];
unsigned long structureSignature; //0x61417272
unsigned long freeClusterCount; //initial: 0xffffffff
unsigned long nextFreeCluster; //initial: 0xffffffff
uint8_t reserved2[12];
unsigned long trailSignature; //0xaa550000
};

//Structure to access Directory Entry in the FAT
struct dir_Structure{
uint8_t name[11];
uint8_t attrib; //file attributes
uint8_t NTreserved; //always 0
uint8_t timeTenth; //tenths of seconds, set to 0 here
unsigned int createTime; //time file was created
unsigned int createDate; //date file was created
unsigned int lastAccessDate;
unsigned int firstClusterHI; //higher word of the first cluster number
unsigned int writeTime; //time of last write
unsigned int writeDate; //date of last write
unsigned int firstClusterLO; //lower word of the first cluster number
unsigned long fileSize; //size of file in bytes
};

//Attribute definitions for file/directory
#define ATTR_READ_ONLY     0x01
#define ATTR_HIDDEN        0x02
#define ATTR_SYSTEM        0x04
#define ATTR_VOLUME_ID     0x08
#define ATTR_DIRECTORY     0x10
#define ATTR_ARCHIVE       0x20
#define ATTR_LONG_NAME     0x0f


#define DIR_ENTRY_SIZE     0x32
#define EMPTY              0x00
#define DELETED            0xe5
#define GET     0
#define SET     1
#define READ	0
#define VERIFY  1
#define ADD		0
#define REMOVE	1
#define LOW		0
#define HIGH	1	
#define TOTAL_FREE   1
#define NEXT_FREE    2
#define GET_LIST     0
#define GET_FILE     1
#define DELETE		 2
#define EOF		0x0fffffff


#define MAX_STRING_SIZE		100	 //defining the maximum size of the dataString


//************* external variables *************
volatile unsigned long firstDataSector, rootCluster, totalClusters;
volatile unsigned int  bytesPerSector, sectorPerCluster, reservedSectorCount;
unsigned long unusedSectors, appendFileSector, appendFileLocation, fileSize, appendStartCluster;

//global flag to keep track of free cluster count updating in FSinfo sector
uint8_t freeClusterCountUpdated;

//data string where data is collected before sending to the card
volatile uint8_t dataString[MAX_STRING_SIZE];



//************* functions *************
uint8_t getBootSectorData (void);
unsigned long getFirstSector(unsigned long clusterNumber);
unsigned long getSetFreeCluster(uint8_t totOrNext, uint8_t get_set, unsigned long FSEntry);
struct dir_Structure* findFiles (uint8_t flag, uint8_t *fileName);
unsigned long getSetNextCluster (unsigned long clusterNumber,uint8_t get_set,unsigned long clusterEntry);
uint8_t readFile (uint8_t flag, uint8_t *fileName);
uint8_t convertFileName (uint8_t *fileName);
uint8_t writeFile (uint8_t *fileName);
void appendFile (void);
unsigned long searchNextFreeCluster (unsigned long startCluster);
void displayMemory (uint8_t flag, unsigned long memory);
void deleteFile (uint8_t *fileName);
void freeMemoryUpdate (uint8_t flag, unsigned long size);

#endif
