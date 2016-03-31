#include "stdafx.h"
#include <cstring>

void SaveImage(char* szPathName, void* lpBits, int w, int h) {
	FILE *pFile = fopen(szPathName, "wb");
	fwrite(lpBits, 1, w*h, pFile);
	fclose(pFile);
}

class AceType {
	FT_Library  library;

public:
	int advance;
	int glyph_index;

	FT_Face face;

	AceType(const char *fontpath) {
		FT_Init_FreeType(&library);
		FT_New_Face(library, fontpath, 0, &face);
	}

	~AceType() {
		FT_Done_Face(face);
		FT_Done_FreeType(library);
	}

	FT_Bitmap* draw(char c, int height) {
		FT_Set_Pixel_Sizes(face, 0, height);

		glyph_index = FT_Get_Char_Index(face, c);

		FT_Load_Glyph(face, glyph_index, 0);

		FT_Render_Glyph(face->glyph, FT_RENDER_MODE_NORMAL);

		advance = face->glyph->metrics.horiAdvance >> 6;

		return &face->glyph->bitmap;
	}
};

class CharAtlas {
public:
	int width, height;
	char* buffer;

	CharAtlas(FT_Bitmap *bitmap) {
		width = bitmap->width;
		height = bitmap->rows;

		unsigned int size = width * height;

		buffer = new char[size];

		std::memcpy(buffer, bitmap->buffer, size);
	}
};

class Atlas {
public:
	int atlasWidth, atlasHeight;

	int charX[128];
	int charWidth[128];
	int charHeight[128];
	int charGlyphIndex[128];

	int charAdvance[128];

	int charBitmapTop[128];
	int charBitmapLeft[128];

	char* buffer;
	AceType* ace;

private:
	int charCount;
	int next;
	CharAtlas ** charatlases;
	char* alpha;
	char *fontpath;

public:
	int getKerning(char a, char b) {
		int i1 = charGlyphIndex[a];
		int i2 = charGlyphIndex[b];

		FT_Vector delta;

		FT_Get_Kerning(ace->face, i1, i2, FT_KERNING_DEFAULT, &delta);

		return delta.x >> 6;
	}

	Atlas(char* fontpath) {
		this->fontpath = fontpath;
		alpha = " 0123456789abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ`~!@#$%^&*()-_=+[{]}\\|;:'\",<.>/?";
		next = 0;
		charCount = std::strlen(alpha);
		charatlases = new CharAtlas*[charCount];
		atlasWidth = atlasHeight = 0;
		buffer = 0;

		for (int i = 0; i<charCount; i++)
			charatlases[i] = 0;

		ace = new AceType(fontpath);
	}

	~Atlas() {
		for (int i = 0; i<charCount; i++) {
			if (charatlases[i]) delete charatlases[i];
		}

		delete[] charatlases;

		if (buffer)
			delete[] buffer;

		delete ace;
	}

public:
	void load(int height) {
		for (int i = 0; i<charCount; i++) {
			char c = alpha[i];

			FT_Bitmap *bitmap = ace->draw(c, height);

			charGlyphIndex[c] = ace->glyph_index;
			charAdvance[c] = ace->advance;
			charBitmapTop[c] = ace->face->glyph->bitmap_top;
			charBitmapLeft[c] = ace->face->glyph->bitmap_left;

			charWidth[c] = bitmap->width;
			charHeight[c] = bitmap->rows;

			add(c, bitmap);
		}

		allocate();
	}

	void toFile(char* filename) {
		SaveImage(filename, buffer, atlasWidth, atlasHeight);
	}

private:

	int getMaxHeight() {
		int max = -1;

		for (int i = 0; i<charCount; i++) {
			CharAtlas * charatlas = charatlases[i];
			if (charatlas->height > max) max = charatlas->height;
		}

		return max;
	}

	void copy(CharAtlas * charatlas, char* buffer, int bufferWidth, int startX, int height) {
		for (int y = 0; y<height; y++) {
			for (int x = 0; x<charatlas->width; x++) {
				int bufferMem = (startX + x) + y * bufferWidth;

				if (y >= charatlas->height) {
					buffer[bufferMem] = 0;
					continue;
				}

				int mem = x + y * charatlas->width;
				int data = charatlas->buffer[mem];

				buffer[bufferMem] = data;
			}
		}
	}

	void allocate() {
		atlasHeight = getMaxHeight();

		buffer = new char[atlasWidth * atlasHeight];
		memset(buffer, 0, atlasWidth * atlasHeight);

		int x = 0;

		for (int i = 0; i<charCount; i++) {
			CharAtlas * charatlas = charatlases[i];

			copy(charatlas, buffer, atlasWidth, x, atlasHeight);

			x += charatlas->width;
		}
	}

	void add(char c, FT_Bitmap *bitmap) {
		charX[c] = atlasWidth;

		atlasWidth += bitmap->width;

		CharAtlas *charatlas = new CharAtlas(bitmap);

		charatlases[next++] = charatlas;
	}
};

class CoreDraw {
public:
	void core(char* str, int x, int y, Atlas* atlas, int* coords) {
		int len = strlen(str);

		int index = 0;

		int penX = x, penY;

		for (int i = 0; i < len; i++) {
			char c = str[i];

			if (c == ' ') {
				penX += atlas->charAdvance[c];
				index += 2;
				continue;
			}

			penY = y - atlas->charBitmapTop[c];

			int nudgeX = atlas->charBitmapLeft[c];

			int a = penX + nudgeX, b = penY;
			coords[index++] = a;
			coords[index++] = b;

			if (i < len - 1) {
				penX += atlas->getKerning(c, str[i + 1]) + atlas->charAdvance[c];
			}
		}
	}
};

class Canvas {
public:
	int w, h;
	char* buffer;

	Canvas(int width, int height) {
		w = width;
		h = height;

		buffer = new char[w * h];

		memset(buffer, 0, w * h);
	}

	~Canvas() {
		delete[]buffer;
	}

	void draw(char *s, int x, int y, Atlas* atlas) {
		CoreDraw core;

		int *coords = new int[strlen(s) * 2];

		core.core(s, x, y, atlas, coords);

		int len = strlen(s);

		for (int i = 0; i < len; ++i) {

			int px = coords[2 * i];
			int py = coords[2 * i + 1];

			draw(s[i], px, py, atlas);
		}

		delete[] coords;

		for (int i = 0; i<640; i++) {
			buffer[i + y * 640] = 255;
		}
	}

	void draw(char c, int x, int y, Atlas*atlas) {
		int cw = atlas->charWidth[c];
		int ch = atlas->charHeight[c];
		int cx = atlas->charX[c];

		int wCanvas = w;

		for (int px = x; px<x + cw; px++) {
			if (px<0) continue;

			for (int py = y; py<y + ch; py++) {
				if (py<0) continue;

				int xCanvas = px;
				int yCanvas = py;

				int memCanvas = xCanvas + yCanvas * wCanvas;

				int xAtlas = cx + (px - x);
				int yAtlas = py - y;

				int memAtlas = xAtlas + yAtlas * atlas->atlasWidth;

				buffer[memCanvas] = atlas->buffer[memAtlas];
			}
		}
	}

	void toFile(char* filename) {
		SaveImage(filename, buffer, w, h);
	}
};

