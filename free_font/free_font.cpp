#include "stdafx.h"
#include "FreeFont.h"

void SaveImage(char* szPathName, void* lpBits, int w, int h) {
	FILE *pFile = fopen(szPathName, "wb");
	fwrite(lpBits, 1, w*h, pFile);
	fclose(pFile);
}

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

