#include <nds.h>
#include <fat.h>
#include <stdio.h>
#include <maxmod9.h>

#include "common/dsimenusettings.h"
#include "common/systemdetails.h"
#include "common/gl2d.h"
#include "graphics/lodepng.h"
#include "graphics/graphics.h"
#include "sound.h"

#include "logo_twlmenupp.h"
//#include "logo_anniversary.h"
//#include "logoPhat_anniversary.h"
#include "block_transparent.h"
#include "icon_nds.h"
#include "icon_ndsl.h"
#include "icon_ndsi.h"
#include "icon_gba.h"
#include "iconPhat_gba.h"
#include "icon_gb.h"
#include "icon_a26.h"
#include "icon_int.h"
#include "icon_nes.h"
#include "icon_sms.h"
#include "icon_gg.h"
#include "icon_pce.h"
#include "icon_md.h"
#include "icon_snes.h"
#include "icon_present.h"

extern bool useTwlCfg;

//extern void loadROMselectAsynch(void);

extern u16 convertVramColorToGrayscale(u16 val);

static int twlmenuTexID;
//static int anniversaryTexID;
static int transparentTexID;
static int ndsTexID;
static int gbaTexID;
static int gbTexID;
static int a26TexID;
static int intTexID;
static int nesTexID;
static int smsTexID;
static int ggTexID;
static int pceTexID;
static int mdTexID;
static int snesTexID;

static glImage twlmenuText[(128 / 16) * (32 / 16)];
//static glImage anniversaryText[1];
static glImage transparentBlock[1];
static glImage ndsIcon[1];
static glImage gbaIcon[1];
static glImage gbIcon[1];
static glImage a26Icon[1];
static glImage intIcon[1];
static glImage nesIcon[1];
static glImage smsIcon[1];
static glImage ggIcon[1];
static glImage pceIcon[1];
static glImage mdIcon[1];
static glImage snesIcon[1];

extern u16 frameBuffer[2][256*192];
extern u16 frameBufferBot[2][256*192];
extern bool doubleBuffer;

extern u16 convertToDsBmp(u16 val);

extern bool fadeType;
extern bool fadeColor;
extern bool controlTopBright;
static char videoFrameFilename[256];

static FILE* videoFrameFile;

static int blackCoverSize = 32;
static int twilightCurrentLine = 191;
static int menuCurrentLine = 0;
static int textScale = 191*10;
static bool videoDonePlaying = false;
static bool scaleTwlmText = false;
static bool hideTwlMenuTextSprite = false;
static bool changeBgAlpha = false;

static int frameDelaySprite = 0;
static bool frameDelaySpriteEven = true;	// For 24FPS
static bool loadFrameSprite = true;

/*static int anniversaryTextYpos = -14;
static bool anniversaryTextYposMove = false;
static int anniversaryTextYposMoveSpeed = 9;
static int anniversaryTextYposMoveDelay = 0;
static bool anniversaryTextYposMoveDelayEven = true;	// For 24FPS */

static int zoomingIconXpos[11] = {-32, -32, 256, 256+16, -32, -32, 256, 256+16, -32, -32, 256+16};
static int zoomingIconYpos[11] = {-32, -48, -48, -32, 192+32, 192+48, 192+48, 192+32, -32, 192, -32};

void twlMenuVideo_loadTopGraphics(void) {
	// TWiLight Menu++ text
	glDeleteTextures(1, &twlmenuTexID);
	
	u16* icon_Pal = (u16*)logo_twlmenuppPal;
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	twlmenuTexID =
	glLoadTileSet(twlmenuText, // pointer to glImage array
				128, // sprite width
				16, // sprite height
				128, // bitmap width
				32, // bitmap height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_128, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Load our 16 color tiles palette
				(u8*) logo_twlmenuppBitmap // image data generated by GRIT
				);

	// Anniversary
	/*glDeleteTextures(1, &anniversaryTexID);
	
	icon_Pal = (u16*)(sys().isDSPhat() ? logoPhat_anniversaryPal : logo_anniversaryPal);
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	anniversaryTexID =
	glLoadTileSet(anniversaryText, // pointer to glImage array
				256, // sprite width
				64, // sprite height
				256, // bitmap width
				64, // bitmap height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_256, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_64, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Load our 16 color tiles palette
				(u8*) (sys().isDSPhat() ? logoPhat_anniversaryBitmap : logo_anniversaryBitmap) // image data generated by GRIT
				);*/

	// Transparent block (for bar growth)
	glDeleteTextures(1, &transparentTexID);
	
	transparentTexID =
	glLoadTileSet(transparentBlock, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap width
				32, // bitmap height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
				16, // Length of the palette to use (16 colors)
				(u16*) block_transparentPal, // Load our 2 color tile palette
				(u8*) block_transparentBitmap // image data generated by GRIT
				);

	char currentDate[16];
	time_t Raw;
	time(&Raw);
	const struct tm *Time = localtime(&Raw);

	strftime(currentDate, sizeof(currentDate), "%m/%d", Time);

	bool december = (strncmp(currentDate, "12", 2) == 0);

	// NDS
	glDeleteTextures(1, &ndsTexID);

	u8* icon_Bitmap = (u8*)icon_ndsBitmap;
	icon_Pal = (u16*)icon_ndsPal;
	if (december) {
		icon_Bitmap = (u8*)icon_presentBitmap;
		icon_Pal = (u16*)icon_presentPal;
	} else if (!sys().isRegularDS()) {
		icon_Bitmap = (u8*)icon_ndsiBitmap;
		icon_Pal = (u16*)icon_ndsiPal;
	} else if (!sys().isDSPhat()) {
		icon_Bitmap = (u8*)icon_ndslBitmap;
		icon_Pal = (u16*)icon_ndslPal;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	ndsTexID =
	glLoadTileSet(ndsIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap width
				32, // bitmap height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Load our 16 color tiles palette
				icon_Bitmap // image data generated by GRIT
				);

	// GBA
	glDeleteTextures(1, &gbaTexID);
	
	if (december) {
		icon_Bitmap = (u8*)icon_presentBitmap;
		icon_Pal = (u16*)icon_presentPal;
	} else {
		icon_Bitmap = (u8*)(sys().isDSPhat() ? iconPhat_gbaBitmap : icon_gbaBitmap);
		icon_Pal = (u16*)(sys().isDSPhat() ? iconPhat_gbaPal : icon_gbaPal);
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gbaTexID =
	glLoadTileSet(gbaIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap width
				32, // bitmap height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT, // param for glTexImage2D() in videoGL.h
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Load our 16 color tiles palette
				icon_Bitmap // image data generated by GRIT
				);

	// GBC
	glDeleteTextures(1, &gbTexID);
	
	if (december) {
		icon_Bitmap = (u8*)icon_presentBitmap;
		icon_Pal = (u16*)icon_presentPal;
	} else 	{
		icon_Bitmap = (u8*)icon_gbBitmap;
		icon_Pal = (u16*)icon_gbPal;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	gbTexID =
	glLoadTileSet(gbIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Image palette
				icon_Bitmap // Raw image data
				);

	// A26
	glDeleteTextures(1, &a26TexID);

	if (december) {
		icon_Bitmap = (u8*)icon_presentBitmap;
		icon_Pal = (u16*)icon_presentPal;
	} else 	{
		icon_Bitmap = (u8*)icon_a26Bitmap;
		icon_Pal = (u16*)icon_a26Pal;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	a26TexID =
	glLoadTileSet(a26Icon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Image palette
				icon_Bitmap // Raw image data
				);

	// INT
	glDeleteTextures(1, &intTexID);

	if (december) {
		icon_Bitmap = (u8*)icon_presentBitmap;
		icon_Pal = (u16*)icon_presentPal;
	} else 	{
		icon_Bitmap = (u8*)icon_intBitmap;
		icon_Pal = (u16*)icon_intPal;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	intTexID =
	glLoadTileSet(intIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Image palette
				icon_Bitmap // Raw image data
				);

	// NES
	glDeleteTextures(1, &nesTexID);

	if (december) {
		icon_Bitmap = (u8*)icon_presentBitmap;
		icon_Pal = (u16*)icon_presentPal;
	} else 	{
		icon_Bitmap = (u8*)icon_nesBitmap;
		icon_Pal = (u16*)icon_nesPal;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	nesTexID =
	glLoadTileSet(nesIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Image palette
				icon_Bitmap // Raw image data
				);

	// SMS
	glDeleteTextures(1, &smsTexID);
	
	if (december) {
		icon_Bitmap = (u8*)icon_presentBitmap;
		icon_Pal = (u16*)icon_presentPal;
	} else 	{
		icon_Bitmap = (u8*)icon_smsBitmap;
		icon_Pal = (u16*)icon_smsPal;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	smsTexID =
	glLoadTileSet(smsIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Image palette
				icon_Bitmap // Raw image data
				);

	// GG
	glDeleteTextures(1, &ggTexID);
	
	if (december) {
		icon_Bitmap = (u8*)icon_presentBitmap;
		icon_Pal = (u16*)icon_presentPal;
	} else 	{
		icon_Bitmap = (u8*)icon_ggBitmap;
		icon_Pal = (u16*)icon_ggPal;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	ggTexID =
	glLoadTileSet(ggIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Image palette
				icon_Bitmap // Raw image data
				);

	// PCE
	glDeleteTextures(1, &pceTexID);
	
	if (december) {
		icon_Bitmap = (u8*)icon_presentBitmap;
		icon_Pal = (u16*)icon_presentPal;
	} else 	{
		icon_Bitmap = (u8*)icon_pceBitmap;
		icon_Pal = (u16*)icon_pcePal;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	pceTexID =
	glLoadTileSet(pceIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Image palette
				icon_Bitmap // Raw image data
				);

	// MD
	glDeleteTextures(1, &mdTexID);
	
	if (december) {
		icon_Bitmap = (u8*)icon_presentBitmap;
		icon_Pal = (u16*)icon_presentPal;
	} else 	{
		icon_Bitmap = (u8*)icon_mdBitmap;
		icon_Pal = (u16*)icon_mdPal;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	mdTexID =
	glLoadTileSet(mdIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Image palette
				icon_Bitmap // Raw image data
				);

	// SNES
	glDeleteTextures(1, &snesTexID);
	
	if (december) {
		icon_Bitmap = (u8*)icon_presentBitmap;
		icon_Pal = (u16*)icon_presentPal;
	} else 	{
		icon_Bitmap = (u8*)icon_snesBitmap;
		icon_Pal = (u16*)icon_snesPal;
	}
	if (ms().colorMode == 1) {
		for (int i2 = 0; i2 < 16; i2++) {
			*(icon_Pal+i2) = convertVramColorToGrayscale(*(icon_Pal+i2));
		}
	}
	snesTexID =
	glLoadTileSet(snesIcon, // pointer to glImage array
				32, // sprite width
				32, // sprite height
				32, // bitmap image width
				32, // bitmap image height
				GL_RGB16, // texture type for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeX for glTexImage2D() in videoGL.h
				TEXTURE_SIZE_32, // sizeY for glTexImage2D() in videoGL.h
				TEXGEN_OFF | GL_TEXTURE_COLOR0_TRANSPARENT,
				16, // Length of the palette to use (16 colors)
				icon_Pal, // Image palette
				icon_Bitmap // Raw image data
				);
}

extern char soundBank[];
extern bool soundBankInited;
mm_sound_effect bootJingle;

void twlMenuVideo_topGraphicRender(void) {
	if (twilightCurrentLine < 0 && menuCurrentLine > 191) {
		videoDonePlaying = true;
	}
	if (videoDonePlaying) {
	  if (scaleTwlmText) {
		if (textScale < 32) {
			textScale -= 2;
			REG_BLDY = 0;
		} else if (textScale < 64) {
			textScale -= 4;
			REG_BLDY = 0;
		} else if (textScale < 96) {
			textScale -= 8;
			REG_BLDY = (0b0001 << 1);
		} else if (textScale < 128) {
			textScale -= 16;
			REG_BLDY = (0b0010 << 1);
		} else if (textScale < 160) {
			textScale -= 24;
			REG_BLDY = (0b0011 << 1);
		} else if (textScale < 192) {
			textScale -= 32;
			REG_BLDY = (0b0011 << 1);
		} else {
			textScale -= 48;
			REG_BLDY = changeBgAlpha ? (0b0100 << 1) : 0;
		}
		if (textScale < 16) textScale = 16;
	  }
	} else {
		for (int y = 0; y < 94; y++) {
			dmaCopyWordsAsynch(0, (u16*)frameBuffer[0]+(256*twilightCurrentLine), (u16*)BG_GFX+(256*y), 0x200);
			dmaCopyWordsAsynch(1, (u16*)frameBuffer[1]+(256*menuCurrentLine), (u16*)BG_GFX+(256*(191-y)), 0x200);
			if (y==93) {
				while (dmaBusy(1));
				dmaCopyWordsAsynch(1, (u16*)frameBuffer[1]+(256*menuCurrentLine), (u16*)BG_GFX+(256*(191-y-1)), 0x200);
			}
			while (dmaBusy(0) || dmaBusy(1));
		}
		twilightCurrentLine--;
		menuCurrentLine++;
		blackCoverSize--;
		if (blackCoverSize < -32) blackCoverSize = -32;
	}

	glBegin2D();
	{
		glColor(RGB15(31, 31-(3*ms().blfLevel), 31-(6*ms().blfLevel)));
		//glSprite(0, anniversaryTextYpos, GL_FLIP_NONE, anniversaryText);
		if (videoDonePlaying && !hideTwlMenuTextSprite) {
			glSpriteStretchVertical(68, (78-textScale)+16, textScale, &twlmenuText[0]);
			glSpriteStretchVertical(68, 97, textScale, &twlmenuText[1]);
		}
		if (blackCoverSize > -32) {
			glBoxFilled(0, 0, 256, blackCoverSize, RGB15(0, 0, 0));
			for (int i = -16; i < 256+16; i += 32) {
				glSprite(i, blackCoverSize-16, GL_FLIP_NONE, transparentBlock);
			}
			glBoxFilled(0, 191-blackCoverSize, 256, 192, RGB15(0, 0, 0));
			for (int i = -16; i < 256+16; i += 32) {
				glSprite(i, 191-(blackCoverSize+15), GL_FLIP_NONE, transparentBlock);
			}
		}
	if (scaleTwlmText) {
		glSprite(zoomingIconXpos[0], zoomingIconYpos[0], GL_FLIP_NONE, nesIcon);
		glSprite(zoomingIconXpos[1], zoomingIconYpos[1], GL_FLIP_NONE, gbIcon);
		glSprite(zoomingIconXpos[2], zoomingIconYpos[2], GL_FLIP_NONE, snesIcon);
		glSprite(zoomingIconXpos[3], zoomingIconYpos[3], GL_FLIP_NONE, gbaIcon);
		glSprite(zoomingIconXpos[4], zoomingIconYpos[4], GL_FLIP_NONE, smsIcon);
		glSprite(zoomingIconXpos[5], zoomingIconYpos[5], GL_FLIP_NONE, mdIcon);
		glSprite(zoomingIconXpos[6], zoomingIconYpos[6], GL_FLIP_NONE, ggIcon);
		glSprite(zoomingIconXpos[7], zoomingIconYpos[7], GL_FLIP_NONE, ndsIcon);
		glSprite(zoomingIconXpos[8], zoomingIconYpos[8], GL_FLIP_NONE, a26Icon);
		glSprite(zoomingIconXpos[9], zoomingIconYpos[9], GL_FLIP_NONE, pceIcon);
		glSprite(zoomingIconXpos[10], zoomingIconYpos[10], GL_FLIP_NONE, intIcon);
	}
		//glBoxFilled(0, 0, 256, 23, RGB15(0, 0, 0));
		//glBoxFilled(0, 168, 256, 192, RGB15(0, 0, 0));
	}
	glEnd2D();
	GFX_FLUSH = 0;

	if (!loadFrameSprite) {
		frameDelaySprite++;
		loadFrameSprite = (frameDelaySprite == 2+frameDelaySpriteEven);
	}

	if (loadFrameSprite && scaleTwlmText && textScale < 191*5) {
		zoomingIconXpos[0] += 4;
		zoomingIconYpos[0] += 5;
		if (zoomingIconXpos[0] > 24) {
			zoomingIconXpos[0] = 24;
		}
		if (zoomingIconYpos[0] > 56) {
			zoomingIconYpos[0] = 56;
		}

		zoomingIconXpos[1] += 6;
		zoomingIconYpos[1] += 4;
		if (zoomingIconXpos[1] > 80) {
			zoomingIconXpos[1] = 80;
		}
		if (zoomingIconYpos[1] > 12) {
			zoomingIconYpos[1] = 12;
		}

		zoomingIconXpos[2] -= 6;
		zoomingIconYpos[2] += 4;
		if (zoomingIconXpos[2] < 154) {
			zoomingIconXpos[2] = 154;
		}
		if (zoomingIconYpos[2] > 12) {
			zoomingIconYpos[2] = 12;
		}

		zoomingIconXpos[3] -= 4;
		zoomingIconYpos[3] += 4;
		if (zoomingIconXpos[3] < 202) {
			zoomingIconXpos[3] = 202;
		}
		if (zoomingIconYpos[3] > 44) {
			zoomingIconYpos[3] = 44;
		}

		zoomingIconXpos[4] += 4;
		zoomingIconYpos[4] -= 5;
		if (zoomingIconXpos[4] > 32) {
			zoomingIconXpos[4] = 32;
		}
		if (zoomingIconYpos[4] < 120) {
			zoomingIconYpos[4] = 120;
		}

		zoomingIconXpos[5] += 6;
		zoomingIconYpos[5] -= 4;
		if (zoomingIconXpos[5] > 80) {
			zoomingIconXpos[5] = 80;
		}
		if (zoomingIconYpos[5] < 152) {
			zoomingIconYpos[5] = 152;
		}

		zoomingIconXpos[6] -= 6;
		zoomingIconYpos[6] -= 5;
		if (zoomingIconXpos[6] < 150) {
			zoomingIconXpos[6] = 150;
		}
		if (zoomingIconYpos[6] < 142) {
			zoomingIconYpos[6] = 142;
		}

		zoomingIconXpos[7] -= 4;
		zoomingIconYpos[7] -= 5;
		if (zoomingIconXpos[7] < 202) {
			zoomingIconXpos[7] = 202;
		}
		if (zoomingIconYpos[7] < 120) {
			zoomingIconYpos[7] = 120;
		}

		zoomingIconXpos[8] += 4;
		zoomingIconYpos[8] += 4;
		if (zoomingIconXpos[8] > 8) {
			zoomingIconXpos[8] = 8;
		}
		if (zoomingIconYpos[8] > 8) {
			zoomingIconYpos[8] = 8;
		}

		zoomingIconXpos[9] += 4;
		zoomingIconYpos[9] -= 4;
		if (zoomingIconXpos[9] > 8) {
			zoomingIconXpos[9] = 8;
		}
		if (zoomingIconYpos[9] < 192-40) {
			zoomingIconYpos[9] = 192-40;
		}

		zoomingIconXpos[10] -= 4;
		zoomingIconYpos[10] += 4;
		if (zoomingIconXpos[10] < 256-40) {
			zoomingIconXpos[10] = 256-40;
		}
		if (zoomingIconYpos[10] > 8) {
			zoomingIconYpos[10] = 8;
		}

		frameDelaySprite = 0;
		frameDelaySpriteEven = !frameDelaySpriteEven;
		loadFrameSprite = false;
	}
	
	/*if (rocketVideo_playVideo && rocketVideo_currentFrame >= 13) {
		if (!anniversaryTextYposMove) {
			anniversaryTextYposMoveDelay++;
			anniversaryTextYposMove = (anniversaryTextYposMoveDelay == 2+anniversaryTextYposMoveDelayEven);
		}

		if (anniversaryTextYposMove && anniversaryTextYpos < 40) {
			anniversaryTextYpos += anniversaryTextYposMoveSpeed;
			anniversaryTextYposMoveSpeed--;
			if (anniversaryTextYposMoveSpeed < 1) anniversaryTextYposMoveSpeed = 1;

			anniversaryTextYposMoveDelay = 0;
			anniversaryTextYposMoveDelayEven = !anniversaryTextYposMoveDelayEven;
			anniversaryTextYposMove = false;
		}
	}*/
}

void twlMenuVideo(void) {
	extern bool twlMenuSplash;
	twlMenuSplash = true;
	//dmaFillHalfWords(0, BG_GFX, 0x18000);

	snd();
	snd().beginStream();
	
	char currentDate[16], bgPath[256], logoPath[256];
	time_t Raw;
	time(&Raw);
	const struct tm *Time = localtime(&Raw);

	strftime(currentDate, sizeof(currentDate), "%m/%d", Time);

	if (strncmp(currentDate, "12", 2) == 0) {
		// Load christmas BG for December
		sprintf(bgPath, "nitro:/graphics/bg_twlmenuppXmas.png");
		sprintf(logoPath, "nitro:/graphics/logo_twlmenuppXmas.png");
	} else if (strcmp(currentDate, "10/31") == 0) {
		// Load orange BG for Halloween
		sprintf(bgPath, "nitro:/graphics/bg_twlmenuppOrange.png");
		sprintf(logoPath, "nitro:/graphics/logo_twlmenuppOrange.png");
	} else if (strcmp(currentDate, "02/14") == 0) {
		// Load pink BG for Valentine's Day
		sprintf(bgPath, "nitro:/graphics/bg_twlmenuppPink.png");
		sprintf(logoPath, "nitro:/graphics/logo_twlmenuppPink.png");
	} else if (strcmp(currentDate, "03/17") == 0 || strcmp(currentDate, "04/22") == 0) {
		// Load green BG for St. Patrick's Day, or Earth Day
		sprintf(bgPath, "nitro:/graphics/bg_twlmenuppGreen.png");
		sprintf(logoPath, "nitro:/graphics/logo_twlmenuppGreen.png");
	} else {
		// Load normal BG
		sprintf(bgPath, "nitro:/graphics/bg_twlmenupp.png");
		sprintf(logoPath, "nitro:/graphics/logo_twlmenupp.png");
	}

	while (!videoDonePlaying)
	{
		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT) || (keysHeld() & KEY_TOUCH)) return;
		//loadROMselectAsynch();
		snd().updateStream();
		swiWaitForVBlank();
	}
	while (dmaBusy(0) || dmaBusy(1));

	controlTopBright = false;
	fadeColor = false;
	fadeType = false;

	while (!screenFadedOut()) { swiWaitForVBlank(); }

	// Load RocketRobz logo
	std::vector<unsigned char> image;
	unsigned width, height;
	lodepng::decode(image, width, height, sys().isDSPhat() ? "nitro:/graphics/logoPhat_rocketrobz.png" : "nitro:/graphics/logo_rocketrobz.png");
	bool alternatePixel = false;
	for(unsigned i=0;i<image.size()/4;i++) {
		image[(i*4)+3] = 0;
		if (alternatePixel) {
			if (image[(i*4)] >= 0x4) {
				image[(i*4)] -= 0x4;
				image[(i*4)+3] |= BIT(0);
			}
			if (image[(i*4)+1] >= 0x4) {
				image[(i*4)+1] -= 0x4;
				image[(i*4)+3] |= BIT(1);
			}
			if (image[(i*4)+2] >= 0x4) {
				image[(i*4)+2] -= 0x4;
				image[(i*4)+3] |= BIT(2);
			}
		}
		u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			color = convertVramColorToGrayscale(color);
		}
		frameBufferBot[0][i] = color;
		if (alternatePixel) {
			if (image[(i*4)+3] & BIT(0)) {
				image[(i*4)] += 0x4;
			}
			if (image[(i*4)+3] & BIT(1)) {
				image[(i*4)+1] += 0x4;
			}
			if (image[(i*4)+3] & BIT(2)) {
				image[(i*4)+2] += 0x4;
			}
		} else {
			if (image[(i*4)] >= 0x4) {
				image[(i*4)] -= 0x4;
			}
			if (image[(i*4)+1] >= 0x4) {
				image[(i*4)+1] -= 0x4;
			}
			if (image[(i*4)+2] >= 0x4) {
				image[(i*4)+2] -= 0x4;
			}
		}
		color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			color = convertVramColorToGrayscale(color);
		}
		frameBufferBot[1][i] = color;
		if ((i % 256) == 255) alternatePixel = !alternatePixel;
		alternatePixel = !alternatePixel;
	}
	image.clear();

	dmaFillHalfWords(0, frameBuffer[0], 0x18000);
	dmaFillHalfWords(0, frameBuffer[1], 0x18000);

	scaleTwlmText = true;
	doubleBuffer = true;
	fadeType = true;
	changeBgAlpha = true;

	// Load TWLMenu++ BG
	lodepng::decode(image, width, height, bgPath);
	alternatePixel = false;
	for(unsigned i=0;i<image.size()/4;i++) {
		image[(i*4)+3] = 0;
		if (alternatePixel) {
			if (image[(i*4)] >= 0x4) {
				image[(i*4)] -= 0x4;
				image[(i*4)+3] |= BIT(0);
			}
			if (image[(i*4)+1] >= 0x4) {
				image[(i*4)+1] -= 0x4;
				image[(i*4)+3] |= BIT(1);
			}
			if (image[(i*4)+2] >= 0x4) {
				image[(i*4)+2] -= 0x4;
				image[(i*4)+3] |= BIT(2);
			}
		}
		u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			color = convertVramColorToGrayscale(color);
		}
		frameBuffer[0][i] = color;
		if (alternatePixel) {
			if (image[(i*4)+3] & BIT(0)) {
				image[(i*4)] += 0x4;
			}
			if (image[(i*4)+3] & BIT(1)) {
				image[(i*4)+1] += 0x4;
			}
			if (image[(i*4)+3] & BIT(2)) {
				image[(i*4)+2] += 0x4;
			}
		} else {
			if (image[(i*4)] >= 0x4) {
				image[(i*4)] -= 0x4;
			}
			if (image[(i*4)+1] >= 0x4) {
				image[(i*4)+1] -= 0x4;
			}
			if (image[(i*4)+2] >= 0x4) {
				image[(i*4)+2] -= 0x4;
			}
		}
		color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			color = convertVramColorToGrayscale(color);
		}
		frameBuffer[1][i] = color;
		if ((i % 256) == 255) alternatePixel = !alternatePixel;
		alternatePixel = !alternatePixel;
	}
	image.clear();

	while (textScale != 16)
	{
		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT) || (keysHeld() & KEY_TOUCH)) return;
		//loadROMselectAsynch();
		snd().updateStream();
		swiWaitForVBlank();
	}

	// Load TWLMenu++ logo
	lodepng::decode(image, width, height, logoPath);
	alternatePixel = false;
	for(unsigned i=0;i<image.size()/4;i++) {
		image[(i*4)+3] = 0;
		if (alternatePixel) {
			if (image[(i*4)] >= 0x4) {
				image[(i*4)] -= 0x4;
				image[(i*4)+3] |= BIT(0);
			}
			if (image[(i*4)+1] >= 0x4) {
				image[(i*4)+1] -= 0x4;
				image[(i*4)+3] |= BIT(1);
			}
			if (image[(i*4)+2] >= 0x4) {
				image[(i*4)+2] -= 0x4;
				image[(i*4)+3] |= BIT(2);
			}
		}
		u16 color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			color = convertVramColorToGrayscale(color);
		}
		frameBuffer[0][i] = color;
		if (alternatePixel) {
			if (image[(i*4)+3] & BIT(0)) {
				image[(i*4)] += 0x4;
			}
			if (image[(i*4)+3] & BIT(1)) {
				image[(i*4)+1] += 0x4;
			}
			if (image[(i*4)+3] & BIT(2)) {
				image[(i*4)+2] += 0x4;
			}
		} else {
			if (image[(i*4)] >= 0x4) {
				image[(i*4)] -= 0x4;
			}
			if (image[(i*4)+1] >= 0x4) {
				image[(i*4)+1] -= 0x4;
			}
			if (image[(i*4)+2] >= 0x4) {
				image[(i*4)+2] -= 0x4;
			}
		}
		color = image[i*4]>>3 | (image[(i*4)+1]>>3)<<5 | (image[(i*4)+2]>>3)<<10 | BIT(15);
		if (ms().colorMode == 1) {
			color = convertVramColorToGrayscale(color);
		}
		frameBuffer[1][i] = color;
		if ((i % 256) == 255) alternatePixel = !alternatePixel;
		alternatePixel = !alternatePixel;
	}

	hideTwlMenuTextSprite = true;

	u16* twlTextBuffer = new u16[62*14];

	// Change TWL letters to user color
	snprintf(videoFrameFilename, sizeof(videoFrameFilename), "nitro:/graphics/TWL_%i.bmp", (int)(useTwlCfg ? *(u8*)0x02000444 : PersonalData->theme));
	videoFrameFile = fopen(videoFrameFilename, "rb");

	if (videoFrameFile) {
		// Start loading
		fseek(videoFrameFile, 0xe, SEEK_SET);
		u8 pixelStart = (u8)fgetc(videoFrameFile) + 0xe;
		fseek(videoFrameFile, pixelStart, SEEK_SET);
		fread(twlTextBuffer, 1, 0x800, videoFrameFile);
		u16* src = twlTextBuffer;
		int x = 68;
		int y = 93;
		for (int i=0; i<62*14; i++) {
			if (x >= 130) {
				x = 68;
				y--;
			}
			u16 val = *(src++);
			if (val != 0x7C1F && val != 0xFC1F) {
				frameBuffer[0][y*256+x] = convertToDsBmp(val);
				frameBuffer[1][y*256+x] = frameBuffer[0][y*256+x];
			}
			x++;
		}
	}
	fclose(videoFrameFile);

	delete[] twlTextBuffer;

	for (int i = 0; i < (60 * 2)+30; i++)
	{
		scanKeys();
		if ((keysHeld() & KEY_START) || (keysHeld() & KEY_SELECT) || (keysHeld() & KEY_TOUCH)) return;
		//loadROMselectAsynch();
		snd().updateStream();
		swiWaitForVBlank();
	}
}