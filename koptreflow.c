/*
 ** koptreflow.c  page reflow api for koreader.
 **
 **
 ** Copyright (C) 2012  http://willus.com
 **
 ** This program is free software: you can redistribute it and/or modify
 ** it under the terms of the GNU Affero General Public License as
 ** published by the Free Software Foundation, either version 3 of the
 ** License, or (at your option) any later version.
 **
 ** This program is distributed in the hope that it will be useful,
 ** but WITHOUT ANY WARRANTY; without even the implied warranty of
 ** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 ** GNU Affero General Public License for more details.
 **
 ** You should have received a copy of the GNU Affero General Public License
 ** along with this program.  If not, see <http://www.gnu.org/licenses/>.
 **
 */

/*
 ** In willus.h, search for "THIRD PARTY" and then comment out all third
 ** party library macros, e.g. comment out HAVE_Z_LIB, HAVE_PNG_LIB, etc.
 **
 ** In k2pdfopt.h, uncomment the #define K2PDFOPT_KINDLEPDFVIEWER statement.
 **
 */

#include "setting.h"
#include "koptreflow.h"

void k2pdfopt_reflow_bmp(KOPTContext *kctx) {
	static K2PDFOPT_SETTINGS _k2settings, *k2settings;
	static MASTERINFO _masterinfo, *masterinfo;
	WILLUSBITMAP _srcgrey, *srcgrey;
	WILLUSBITMAP *src, *dst;
	BMPREGION region;
	int i, bw, martop, marbot;

	src = &kctx->src;
	srcgrey = &_srcgrey;
	bmp_init(srcgrey);

	k2settings = &_k2settings;
	masterinfo = &_masterinfo;
	/* Initialize settings */
	k2pdfopt_settings_init_from_koptcontext(k2settings, kctx);
	k2pdfopt_settings_sanity_check(k2settings);
	/* Init master output structure */
	masterinfo_init(masterinfo, k2settings);
	bmp_init(&masterinfo->bmp);
	masterinfo->bmp.width = 0;
	masterinfo->bmp.height = 0;
	wrapbmp_init(&masterinfo->wrapbmp, k2settings->dst_color);
	/* Init new source bitmap */
	masterinfo_new_source_page_init(masterinfo, k2settings, src, srcgrey, NULL,
			&region, k2settings->src_rot, NULL, NULL, 1, NULL);
	/* Process single source page */
	bmpregion_source_page_add(&region, k2settings, masterinfo, 1, 0, (int)(0.25*k2settings->src_dpi+.5));
	wrapbmp_flush(masterinfo,k2settings,0,0);

	if (fabs(k2settings->dst_gamma - 1.0) > .001)
		bmp_gamma_correct(&masterinfo->bmp, &masterinfo->bmp,
				k2settings->dst_gamma);

	/* copy master bitmap to context dst bitmap */
	dst = &kctx->dst;
	martop = (int)(k2settings->dst_dpi*k2settings->dst_martop+.5);
	marbot = (int)(k2settings->dst_dpi*k2settings->dst_marbot+.5);
	dst->bpp = masterinfo->bmp.bpp;
	dst->width = masterinfo->bmp.width;
	dst->height = masterinfo->rows + martop + marbot;
	bmp_alloc(dst);
	bmp_fill(dst,255,255,255);
	bw = bmp_bytewidth(&masterinfo->bmp);
	for (i=0; i<masterinfo->rows; i++)
	    memcpy(bmp_rowptr_from_top(dst,i+martop), bmp_rowptr_from_top(&masterinfo->bmp,i), bw);

	kctx->page_width = kctx->dst.width;
	kctx->page_height = kctx->dst.height;
	kctx->precache = 0;

	bmp_free(src);
	bmp_free(srcgrey);
	masterinfo_free(masterinfo, k2settings);
}
