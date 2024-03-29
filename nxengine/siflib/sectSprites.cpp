
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../nx.h"
#include "../common/DBuffer.h"
#include "../common/bufio.h"

#include "sectSprites.h"
#include "sectSprites.fdh"


int SIFSpritesSect::GetSpriteCount(const uint8_t *data, int datalen)
{
	const uint8_t *data_end = data + (datalen - 1);
	return read_U16(&data, data_end);
}


bool SIFSpritesSect::Decode(const uint8_t *data, int datalen,
      SIFSprite *sprites, int *nsprites_out, int maxsprites)
{
   const uint8_t *data_end = data + (datalen - 1);
   int i, f, nsprites;

   nsprites = read_U16(&data, data_end);
   if (nsprites_out) *nsprites_out = nsprites;

   if (nsprites >= maxsprites)
   {
      NX_ERR("SIFSpritesSect::Decode: too many sprites in file (nsprites=%d, maxsprites=%d)\n", nsprites, maxsprites);
      return 1;
   }

   NX_LOG("SIFSpritesSect: loading %d sprites\n", nsprites);
   for(i=0;i<nsprites;i++)
   {
      if (data > data_end)
      {
         NX_ERR("SIFSpritesSect::Decode: section corrupt: overran end of data\n");
         return 1;
      }

      // read sprite-level fields
      sprites[i].w = read_U8(&data, data_end);
      sprites[i].h = read_U8(&data, data_end);
      sprites[i].spritesheet = read_U8(&data, data_end);

      sprites[i].nframes = read_U8(&data, data_end);
      sprites[i].ndirs = read_U8(&data, data_end);

      if (sprites[i].ndirs > SIF_MAX_DIRS)
      {
         NX_ERR("SIFSpritesSect::Decode: SIF_MAX_DIRS exceeded on sprite %d (ndirs=%d)\n", i, sprites[i].ndirs);
         return 1;
      }

      LoadRect(&sprites[i].bbox, &data, data_end);
      LoadRect(&sprites[i].solidbox, &data, data_end);

      LoadPoint(&sprites[i].spawn_point, &data, data_end);

      LoadPointList(&sprites[i].block_l, &data, data_end);
      LoadPointList(&sprites[i].block_r, &data, data_end);
      LoadPointList(&sprites[i].block_u, &data, data_end);
      LoadPointList(&sprites[i].block_d, &data, data_end);

      // malloc enough space to hold the specified number
      // of apple fritters, i mean, frames.
      sprites[i].frame = (SIFFrame *)malloc(sizeof(SIFFrame) * sprites[i].nframes);

      // then load all frames
      for(f=0;f<sprites[i].nframes;f++)
      {
         if (LoadFrame(&sprites[i].frame[f], sprites[i].ndirs, &data, data_end))
            return 1;
      }
   }

   return 0;
}

bool SIFSpritesSect::LoadFrame(SIFFrame *frame, int ndirs,
      const uint8_t **data, const uint8_t *data_end)
{
   // sets defaults for un-specified/default fields
   memset(frame, 0, sizeof(SIFFrame));

   for(int d=0;d<ndirs;d++)
   {
      SIFDir *dir = &frame->dir[d];
      LoadPoint(&dir->sheet_offset, data, data_end);

      int t;
      for(;;)
      {
         t = read_U8(data, data_end);
         if (t == S_DIR_END) break;

         switch(t)
         {
            case S_DIR_DRAW_POINT: LoadPoint(&dir->drawpoint, data, data_end); break;
            case S_DIR_ACTION_POINT: LoadPoint(&dir->actionpoint, data, data_end); break;
            case S_DIR_ACTION_POINT_2: LoadPoint(&dir->actionpoint2, data, data_end); break;

            case S_DIR_PF_BBOX:
                                       LoadRect(&dir->pf_bbox, data, data_end);
                                       break;

            default:
                                       NX_LOG("SIFSpriteSect::LoadFrame: encountered unknown optional field type %d\n", t);
                                       return 1;
         }
      }
   }

   return 0;
}

void SIFSpritesSect::LoadRect(SIFRect *rect, const uint8_t **data,
      const uint8_t *data_end)
{
	rect->x1 = (int16_t)read_U16(data, data_end);
	rect->y1 = (int16_t)read_U16(data, data_end);
	rect->x2 = (int16_t)read_U16(data, data_end);
	rect->y2 = (int16_t)read_U16(data, data_end);
}

void SIFSpritesSect::LoadPoint(SIFPoint *pt, const uint8_t **data,
      const uint8_t *data_end)
{
	pt->x = (int16_t)read_U16(data, data_end);
	pt->y = (int16_t)read_U16(data, data_end);
}

void SIFSpritesSect::LoadPointList(SIFPointList *lst, const uint8_t **data, const uint8_t *data_end)
{
   lst->count = read_U8(data, data_end);
   if (lst->count > SIF_MAX_BLOCK_POINTS)
   {
      NX_ERR("SIFSpritesSect::LoadPointList: too many block points (%d, max=%d)\n", lst->count, SIF_MAX_BLOCK_POINTS);
      return;
   }

   for(int i=0;i<lst->count;i++)
   {
      lst->point[i].x = (int16_t)read_U16(data, data_end);
      lst->point[i].y = (int16_t)read_U16(data, data_end);
   }
}

uint8_t *SIFSpritesSect::Encode(SIFSprite *sprites,
      int nsprites, int *datalen_out)
{
   DBuffer buf;
   int i, f;

   buf.Append16(nsprites);

   for(i=0;i<nsprites;i++)
   {
      buf.Append8(sprites[i].w);
      buf.Append8(sprites[i].h);
      buf.Append8(sprites[i].spritesheet);

      buf.Append8(sprites[i].nframes);
      buf.Append8(sprites[i].ndirs);

      SaveRect(&sprites[i].bbox, &buf);
      SaveRect(&sprites[i].solidbox, &buf);

      SavePoint(&sprites[i].spawn_point, &buf);

      SavePointList(&sprites[i].block_l, &buf);
      SavePointList(&sprites[i].block_r, &buf);
      SavePointList(&sprites[i].block_u, &buf);
      SavePointList(&sprites[i].block_d, &buf);

      for(f=0;f<sprites[i].nframes;f++)
      {
         SaveFrame(&sprites[i].frame[f], sprites[i].ndirs, &buf);
      }
   }

   if (datalen_out) *datalen_out = buf.Length();
   return buf.TakeData();
}

void SIFSpritesSect::SaveFrame(SIFFrame *frame, int ndirs, DBuffer *out)
{
   for(int d=0;d<ndirs;d++)
   {
      SIFDir *dir = &frame->dir[d];

      SavePoint(&dir->sheet_offset, out);
      SaveOptionalPoint(S_DIR_DRAW_POINT, &dir->drawpoint, out);
      SaveOptionalPoint(S_DIR_ACTION_POINT, &dir->actionpoint, out);
      SaveOptionalPoint(S_DIR_ACTION_POINT_2, &dir->actionpoint2, out);
      SaveOptionalRect(S_DIR_PF_BBOX, &dir->pf_bbox, out);

      out->Append8(S_DIR_END);
   }
}

void SIFSpritesSect::SaveRect(SIFRect *rect, DBuffer *out)
{
	out->Append16((uint16_t)rect->x1);
	out->Append16((uint16_t)rect->y1);
	out->Append16((uint16_t)rect->x2);
	out->Append16((uint16_t)rect->y2);
}

void SIFSpritesSect::SavePoint(SIFPoint *pt, DBuffer *out)
{
	out->Append16((uint16_t)pt->x);
	out->Append16((uint16_t)pt->y);
}

void SIFSpritesSect::SavePointList(SIFPointList *lst, DBuffer *out)
{
	out->Append8(lst->count);
	for(int i=0;i<lst->count;i++)
	{
		out->Append16((uint16_t)lst->point[i].x);
		out->Append16((uint16_t)lst->point[i].y);
	}
}

void SIFSpritesSect::SaveOptionalPoint(int type, SIFPoint *pt, DBuffer *out)
{
	if (!pt->equ(0, 0))
	{
		out->Append8(type);
		SavePoint(pt, out);
	}
}

void SIFSpritesSect::SaveOptionalRect(int type, SIFRect *rect, DBuffer *out)
{
	if (!rect->equ(0, 0, 0, 0))
	{
		out->Append8(type);
		SaveRect(rect, out);
	}
}
