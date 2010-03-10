#ifndef __OE_SWF_H__
#define __OE_SWF_H__

#define SWF_COMPRESSED_MAGIC	"CWS"
#define SWF_UNCOMPRESSED_MAGIC	"FWS"

#include <iostream>
#include <fstream>
#include <vector>

unsigned int GetEncodedU32(std::ifstream *file);

enum SWF_TAGS
{
	TAG_END = 0,
	TAG_SHOW_FRAME,
	TAG_DEFINE_SHAPE,
	TAG_PLACE_OBJECT = 4,
	TAG_REMOVE_OBJECT,
	TAG_DEFINE_BITS,
	TAG_DEFINE_BUTTON,
	TAG_JPEG_TABLES,
	TAG_SET_BACKGROUND_COLOR,
	TAG_DEFINE_FONT,
	TAG_DEFINE_TEXT,
	TAG_DO_ACTION,
	TAG_DEFINE_FONT_INFO,
	TAG_DEFINE_SOUND,
	TAG_START_SOUND,
	TAG_DEFINE_BUTTON_SOUND,
	TAG_SOUND_STREAM_HEAD,
	TAG_SOUND_STREAM_BLOCK,
	TAG_DEFINE_BITS_LOSSLESS,
	TAG_DEFINE_BITS_JPEG2,
	TAG_DEFINE_SHAPE2,
	TAG_DEFINE_BUTTON_CXFORM,
	TAG_PROTECT,
	TAG_PLACE_OBJECT2 = 26,
	TAG_REMOVE_OBJECT2 = 28,
	TAG_DEFINE_SHAPE3 = 32,
	TAG_DEFINE_TEXT2,
	TAG_DEFINE_BUTTON2,
	TAG_DEFINE_BITS_JPEG3,
	TAG_DEFINE_BITS_LOSSLESS2,
	TAG_DEFINE_EDIT_TEXT,
	TAG_DEFINE_SPRITE = 39,
	TAG_FRAME_LABEL = 43,
	TAG_SOUND_STREAM_HEAD2 = 45,
	TAG_DEFINE_MORPH_SHAPE,
	TAG_DEFINE_FONT2 = 48,
	TAG_EXPORT_ASSETS = 56,
	TAG_IMPORT_ASSETS,
	TAG_ENABLE_DEBUGGER,
	TAG_DO_INIT_ACTION,
	TAG_DEFINE_VIDEO_STREAM,
	TAG_VIDEO_FRAME,
	TAG_DEFINE_FONT_INFO2,
	TAG_ENABLE_DEBUGGER2 = 64,
	TAG_SCRIPT_LIMITS,
	TAG_SET_TAB_INDEX,
	TAG_FILE_ATTRIBUTES = 69,
	TAG_PLACE_OBJECT3,
	TAG_IMPORT_ASSETS2,
	TAG_DEFINE_FONT_ALIGN_ZONES = 73,
	TAG_DEFINE_CSM_TEXT_SETTINGS,
	TAG_DEFINE_FONT3,
	TAG_SYMBOL_CLASS,
	TAG_METADATA,
	TAG_DEFINE_SCALING_GRID,
	TAG_DO_ABC = 82,
	TAG_DEFINE_SHAPE4,
	TAG_DEFINE_MORPH_SHAPE2,
	TAG_DEFINE_SCENE_AND_FRAME_LABEL_DATA = 86,
	TAG_DEFINE_BINARY_DATA,
	TAG_DEFINE_FONT_NAME = 88,
	TAG_DEFINE_START_SOUND2 = 89,
	TAG_DEFINE_BITS_JPEG4 = 90,
	TAG_DEFINE_FONT4 = 91
};

struct SWF_TAG
{
	unsigned short tagCodeAndLength;
};

class SWF_FILE
{
public:
	SWF_FILE();
	~SWF_FILE();
	
	unsigned int LoadSWF(const char* path);
	bool IsLoaded();
	
	void GetBytes(char* dataOut, const unsigned int numBytes = 1);
	void GetBits(char* dataOut, const unsigned int numBits);
	
	unsigned int GetBitOffset() { return m_bitOffset; }
	unsigned int GetByteOffset() { return m_byteOffset; }
	void SetByteOffset(const unsigned int offset);
	void SetBitOffset(const unsigned int offset);
private:
	char* m_pFileData;
	unsigned int m_byteOffset;
	unsigned int m_bitOffset;
	unsigned int m_uiFileSize;
	bool m_bLoaded;
};

class SWF_RECT
{
	public:
		SWF_RECT()
		{
			Xmin = 0;
			Xmax = 0;
			Ymin = 0;
			Ymax = 0;
			Nbits = 0;
		};
		~SWF_RECT(){};
		
		unsigned char Nbits;
		
		//	HACK: We're making the assumption that
		//		  the values we get from GetBits are
		//		  not larger than 4 bytes. While it's
		//		  possible to get larger than 4 byte values
		//		  we believe it's highly unlikely.

		int Xmin;
		int Xmax;
		int Ymin;
		int Ymax;
};

struct SWF_FILE_ATTRIBUTES
{
	bool useDirectBlit;
	bool useGPU;
	bool hasMetaData;
	bool useActionScript3;
	bool useNetwork;
};

struct SWF_DEFINE_SCENE_AND_FRAME_LABEL_DATA
{
	unsigned int SceneCount;
	std::vector<unsigned int> FrameOffsets;
	std::vector<std::string> Names;
	unsigned int FrameLabelCount;
	std::vector<unsigned int> FrameNums;
	std::vector<std::string> FrameLabels;
};

struct SWF_HEADER
{
	unsigned char	signature[3];
	unsigned char	version;
	unsigned int	fileLength;
	SWF_RECT*		rect;
	unsigned short	fps;
	unsigned short  numFrames;
};

class SWF
{
	public:
		SWF();
		~SWF();
		unsigned int LoadSWF(const char* path);
		
	private:
		unsigned int LoadHeader(SWF_FILE *file);
		unsigned int LoadTag(std::ifstream *file);
		unsigned int LoadFileAttributesTag(std::ifstream *file);
		unsigned int LoadDefSceneAndFrameLabelTag(std::ifstream *file);
		
		bool m_bIsEnd;
		
		SWF_FILE* m_pFile;
		SWF_HEADER* m_pHeader;
		SWF_FILE_ATTRIBUTES* m_pAttributes;
		SWF_DEFINE_SCENE_AND_FRAME_LABEL_DATA* m_pSceneAndFrameLabelData;
};

#endif /*__OE_SWF_H__*/
