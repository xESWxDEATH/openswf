#include "opeswf.h"

#include <math.h>

SWF_FILE::SWF_FILE()
{
	m_pFileData		= NULL;
	m_byteOffset	= 0;
	m_bitOffset		= 0;
	m_uiFileSize	= 0;
	m_bLoaded		= false;
}

SWF_FILE::~SWF_FILE()
{
	if(m_pFileData)
	{
		delete m_pFileData;
		m_pFileData = NULL;
	}
	
	m_bLoaded = false;
}

unsigned int SWF_FILE::LoadSWF(const char* path)
{
	std::ifstream fSwfFile;
	fSwfFile.open(path, std::ifstream::in | std::ifstream::binary);

	if(!fSwfFile.is_open())
	{
#ifdef _DEBUG
		std::cerr << "Error - Failed to load SWF: \"" << path << "\"" << std::endl;
#endif
		m_bLoaded = false;
		return -1;
	}
	
	fSwfFile.seekg (0, std::ios::end);
	m_uiFileSize = fSwfFile.tellg();
	fSwfFile.seekg (0, std::ios::beg);

	m_pFileData = new char[m_uiFileSize];
	fSwfFile.read(m_pFileData, m_uiFileSize);
	fSwfFile.close();
	
	m_bLoaded = true;
	
	return 0;
}

void SWF_FILE::GetBytes(char* dataOut, const unsigned int numBytes)
{
	memcpy(dataOut, (char*)(m_pFileData+m_byteOffset), numBytes);
	m_byteOffset+=numBytes;
}

void SWF_FILE::GetBits(char* dataOut, const unsigned int numBits)
{
	unsigned int numBytes = (unsigned int)ceil(numBits / 8.0f);
//	memcpy(dataOut, (char*)(m_pFileData+m_byteOffset), numBytes);
	memset(dataOut, 0, numBytes);
	
	int offs = m_bitOffset;
	m_bitOffset += numBits;
	
	if(m_bitOffset>=7)
	{
		m_byteOffset += (unsigned int)ceil(numBits / 8.0f);
		m_bitOffset  =  (m_bitOffset%8);
	}
	else
		m_byteOffset-=numBytes;
	
	char b = m_pFileData[m_byteOffset];
	m_byteOffset++;
	
	char cByte = b;
	int bitsRem = 8 - offs;
	cByte &= (1 << (8 - offs)) - 1;
	cByte >>= offs;
	
	char cBit = 8 - offs - 1;
	char bitCount = 0;
	int destByte = 0;
	while(bitCount < numBits)
	{
		if(bitCount % 7 == 0 && bitCount != 0)
			destByte++;
	
		if(cByte & 0x80)
			dataOut[destByte] |= (bitCount == 0) ? 0x01 : (2 << bitCount);

		cByte <<= 1;
		cByte &= 255;
		cBit--;
		bitCount++;
		
		if(cBit < 0)
		{
			cByte = m_pFileData[m_byteOffset];
			++m_byteOffset;
			cBit = 7;
		}	
	}
}

void SWF_FILE::SetByteOffset(const unsigned int offset)
{
	m_byteOffset = offset;
}

void SWF_FILE::SetBitOffset(const unsigned int offset)
{
	m_bitOffset = offset;
}

bool SWF_FILE::IsLoaded()
{
	return m_bLoaded;
}

//	HACK: This isn't complete.
unsigned int GetEncodedU32(std::ifstream* file)
{
	unsigned char encoded;
	file->read((char*)&encoded, sizeof(unsigned char));
	if((encoded & 0x80) == 0)
		return encoded;
	
	return 0;
}

SWF::SWF()
{
	m_bIsEnd		= false;
	
	m_pFile			= NULL;
	m_pHeader		= NULL;
	m_pAttributes	= NULL;
	m_pSceneAndFrameLabelData = NULL;
}

SWF::~SWF()
{
	if(m_pFile)
	{
		delete m_pFile;
		m_pFile = NULL;
	}

	if(m_pHeader)
	{
		if(m_pHeader->rect)
		{
			delete m_pHeader->rect;
			m_pHeader->rect = NULL;
		}
		delete m_pHeader;
		m_pHeader = NULL;
	}
	
	if(m_pAttributes)
	{
		delete m_pAttributes;
		m_pAttributes = NULL;
	}
	
	if(m_pSceneAndFrameLabelData)
	{
		delete m_pSceneAndFrameLabelData;
		m_pSceneAndFrameLabelData = NULL;
	}
}

unsigned int SWF::LoadSWF(const char* path)
{
	m_pFile = new SWF_FILE;
	m_pFile->LoadSWF(path);
	
	if(!m_pFile->IsLoaded())
		return -1;
	
	LoadHeader(m_pFile);

	//while(!m_bIsEnd)
	//	LoadTag(&fSwfFile);
	
	return 0;
}

//	WARNING: The header's rect member is not being loaded correctly!
//	I've continued working on parsing tags in spite of this.
unsigned int SWF::LoadHeader(SWF_FILE* file)
{
	bool valid = false;
	
	m_pFile->SetByteOffset(0);
	
	m_pHeader = new SWF_HEADER;
	
	m_pFile->GetBytes((char*)&m_pHeader->signature[0]);
	m_pFile->GetBytes((char*)&m_pHeader->signature[1]);
	m_pFile->GetBytes((char*)&m_pHeader->signature[2]);
	
	m_pFile->GetBytes((char*)&m_pHeader->version);
	m_pFile->GetBytes((char*)&m_pHeader->fileLength, sizeof(unsigned int));
	
	m_pHeader->rect = new SWF_RECT;
	
	m_pFile->GetBytes((char*)&m_pHeader->rect->Nbits);
	
	m_pHeader->rect->Nbits >>= 3;
	m_pFile->SetBitOffset(m_pFile->GetBitOffset() + 5);
	m_pFile->SetByteOffset(m_pFile->GetByteOffset() - 1);
	
	unsigned int numBytes = (unsigned int)ceil(m_pHeader->rect->Nbits / 8.0f);
	
	m_pHeader->rect->Xmin = new signed char[numBytes];
	m_pHeader->rect->Xmax = new signed char[numBytes];
	m_pHeader->rect->Ymin = new signed char[numBytes];
	m_pHeader->rect->Ymax = new signed char[numBytes];
	
	m_pFile->GetBits((char*)m_pHeader->rect->Xmin, (unsigned int)m_pHeader->rect->Nbits);
	m_pFile->GetBits((char*)m_pHeader->rect->Xmax, (unsigned int)m_pHeader->rect->Nbits);
	m_pFile->GetBits((char*)m_pHeader->rect->Ymin, (unsigned int)m_pHeader->rect->Nbits);
	m_pFile->GetBits((char*)m_pHeader->rect->Ymax, (unsigned int)m_pHeader->rect->Nbits);
	
//	m_pFile->GetBytes((char*)&m_pHeader->rect->Nbits, sizeof(unsigned char));
//	m_pHeader->rect->Nbits >>= 3;
	
//	file->read((char*)&m_pHeader->rect->Nbits, sizeof(unsigned char));
	
//	m_pHeader->rect->Nbits >>= 3;
	
//	unsigned int remainder = (m_pHeader->rect->Nbits*4) % 8;
//	unsigned int numBytes = remainder == 0 ? (m_pHeader->rect->Nbits*4) / 8 : (m_pHeader->rect->Nbits*4 / 8) + 1;
	
// 	unsigned char* data = new unsigned char[numBytes];
// 	file->read((char*)data, sizeof(unsigned char)*numBytes);
// 	
// 	remainder = (m_pHeader->rect->Nbits) % 8;
// 	numBytes = remainder == 0 ? (m_pHeader->rect->Nbits) / 8 : (m_pHeader->rect->Nbits / 8) + 1;
// 	
// 	m_pHeader->rect->Xmin = new signed char[numBytes];
// 	m_pHeader->rect->Xmax = new signed char[numBytes];
// 	m_pHeader->rect->Ymin = new signed char[numBytes];
// 	m_pHeader->rect->Ymax = new signed char[numBytes];
// 	
// 	*m_pHeader->rect->Xmin = *data;
// 	*m_pHeader->rect->Xmax = *(data+m_pHeader->rect->Nbits);
// 	*m_pHeader->rect->Ymin = *(data+m_pHeader->rect->Nbits*2);
// 	*m_pHeader->rect->Ymax = *(data+m_pHeader->rect->Nbits*3);
// 	
// 	delete data;
// 	delete m_pHeader->rect->Xmin;
// 	delete m_pHeader->rect->Xmax;
// 	delete m_pHeader->rect->Ymin;
// 	delete m_pHeader->rect->Ymax;
// 	
// 	file->read((char*)&m_pHeader->fps, sizeof(unsigned short));
// 	file->read((char*)&m_pHeader->numFrames, sizeof(unsigned short));
	
	return 0;
}

unsigned int SWF::LoadTag(std::ifstream* file)
{
	SWF_TAG tagHeader;
	unsigned short tagType;
	unsigned short tagLength;

	file->read((char*)&tagHeader.tagCodeAndLength, sizeof(unsigned short));

	tagType = tagHeader.tagCodeAndLength >> 6;
	tagLength = tagHeader.tagCodeAndLength & 0x3F;
	unsigned int tagLengthLong = 0;
	bool isLongTag = false;
	if(tagLength == 0x3F)
	{
		isLongTag = true;
		file->read((char*)&tagLengthLong, sizeof(unsigned int));
	}
	
	switch(tagType)
	{
		case TAG_END:
			m_bIsEnd = true;
			break;
		case TAG_SET_BACKGROUND_COLOR:		
			unsigned char red, green, blue;
			file->read((char*)&red, sizeof(char));
			file->read((char*)&green, sizeof(char));
			file->read((char*)&blue, sizeof(char));
#ifdef _DEBUG
			std::cout << "Dumping Tag[SetBackgroundColor]..." << std::endl;
			std::cout << "Tag[FileAttributes]:RGB:Red\t= " << (unsigned int)red << std::endl;
			std::cout << "Tag[FileAttributes]:RGB:Green\t= " << (unsigned int)green << std::endl;
			std::cout << "Tag[FileAttributes]:RGB:Blue\t= " << (unsigned int)blue << std::endl;
#endif
			break;
		case TAG_FILE_ATTRIBUTES:
			LoadFileAttributesTag(file);
			break;
		case TAG_METADATA:
#ifdef _DEBUG
			std::cout << "Dumping Tag[Metadata]..." << std::endl;
#endif
			char* data;
			if(isLongTag)
			{
				data = new char[tagLengthLong];
				file->read((char*)data, sizeof(char)*tagLengthLong);
#ifdef _DEBUG
				std::cout << data << std::endl;
#endif
			}

			delete []data;
			break;
		case TAG_DEFINE_SCENE_AND_FRAME_LABEL_DATA:
			LoadDefSceneAndFrameLabelTag(file);
			break;
		default:
#ifdef _DEBUG
			std::cout << "Unsupported Tag Type:" << tagType << "\t\t" << "Tag Length: " << tagLength;
#endif
			unsigned int pos = file->tellg();

			if(isLongTag)
				file->seekg(pos + tagLengthLong);
			else
				file->seekg(pos + tagLength);
			break;
	}
	
#ifdef _DEBUG	
	std::cout << std::endl;
#endif

	return 0;
}

unsigned int SWF::LoadFileAttributesTag(std::ifstream* file)
{
	m_pAttributes = new SWF_FILE_ATTRIBUTES;

	unsigned int fileAttributes = 0;
	file->read((char*)&fileAttributes, sizeof(unsigned int));
	
	bool useNetwork			= fileAttributes & 0x80;
	bool useActionScript3	= fileAttributes & 0x10;
	bool hasMetaData		= fileAttributes & 0x08;
	bool useGPU				= fileAttributes & 0x04;
	bool useDirectBlit		= fileAttributes & 0x02;
	
	m_pAttributes->useNetwork		= useNetwork;
	m_pAttributes->useActionScript3 = useActionScript3;
	m_pAttributes->hasMetaData		= hasMetaData;
	m_pAttributes->useGPU			= useGPU;
	m_pAttributes->useDirectBlit	= useDirectBlit;
	
#ifdef _DEBUG
	std::cout << "Dumping Tag[FileAttributes]..." << std::endl;
	std::cout << "Tag[FileAttributes]:UseDirectBlit\t= " << useDirectBlit << std::endl;
	std::cout << "Tag[FileAttributes]:UseGPU\t\t= " << useGPU << std::endl;
	std::cout << "Tag[FileAttributes]:HasMetadata\t\t= " << hasMetaData << std::endl;
	std::cout << "Tag[FileAttributes]:ActionScript3\t= " << useActionScript3 << std::endl;
	std::cout << "Tag[FileAttributes]:UseNetwork\t\t= " << useNetwork << std::endl;
	std::cout << std::endl;
#endif

	return 0;
}

//	HACK: This isn't complete.
unsigned int SWF::LoadDefSceneAndFrameLabelTag(std::ifstream* file)
{
	m_pSceneAndFrameLabelData = new SWF_DEFINE_SCENE_AND_FRAME_LABEL_DATA;
	
	m_pSceneAndFrameLabelData->SceneCount = GetEncodedU32(file);
	
	for(unsigned int i = 0; i < m_pSceneAndFrameLabelData->SceneCount; ++i)
	{
		std::string name;
		
		unsigned char charbuff;
		
		m_pSceneAndFrameLabelData->FrameOffsets.push_back(GetEncodedU32(file));
			
		file->read((char*)&charbuff, sizeof(unsigned char));
		while(charbuff != 0)
		{
			name.push_back(charbuff);
			file->read((char*)&charbuff, sizeof(unsigned char));
		}
		
		m_pSceneAndFrameLabelData->Names.push_back(std::string(name));		
	}
	
	m_pSceneAndFrameLabelData->FrameLabelCount = GetEncodedU32(file);
	for(unsigned int i = 0; i < m_pSceneAndFrameLabelData->FrameLabelCount; ++i)
	{
		std::string label;
		
		unsigned char charbuff;
		
		m_pSceneAndFrameLabelData->FrameNums.push_back(GetEncodedU32(file));
		
		file->read((char*)&charbuff, sizeof(unsigned char));
		while(charbuff != 0)
		{
			label.push_back(charbuff);
			file->read((char*)&charbuff, sizeof(unsigned char));
		}
		
		m_pSceneAndFrameLabelData->FrameLabels.push_back(std::string(label));
	}
	
	return 0;
}
