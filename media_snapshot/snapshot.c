#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libswscale/swscale.h>
#include <jpeglib.h>
#include <string.h>
#include <errno.h>
#include <setjmp.h>

#include "internel.h"
#include "snapshot.h"

struct my_error_mgr {
	struct jpeg_error_mgr pub;	/* "public" fields */
	jmp_buf setjmp_buffer;	/* for return to caller */
};

typedef struct my_error_mgr * my_error_ptr;

METHODDEF(void)
my_error_exit (j_common_ptr cinfo)
{
  my_error_ptr myerr = (my_error_ptr) cinfo->err;
  (*cinfo->err->output_message) (cinfo);
  longjmp(myerr->setjmp_buffer, 1);
}

#if 0
static void SaveFrame(AVFrame *pFrame, int width, int height, int iFrame)
{
	FILE *pFile;
	char szFilename[32];
	int y;

	sprintf(szFilename, "frame%d.ppm", iFrame);
	pFile = fopen(szFilename, "wb");
	if( !pFile )
		return;
	fprintf(pFile, "P6\n%d %d\n255\n", width, height);

	for( y = 0; y < height; y++ )
		fwrite(pFrame->data[0] + y * pFrame->linesize[0], 1, width * 3, pFile);

	fclose(pFile);
}
#endif

int test_jpeg(int index, int image_width, int image_height, uint8_t* image_buffer, int quality)
{
	int ret = 0;
	char filename[256];
	sprintf(filename, "frame%d.jpg", index);
	struct jpeg_compress_struct cinfo;
	FILE * outfile;
	JSAMPROW row_pointer[1];
	int row_stride;
	int create_compress = 0;

	if ((outfile = fopen(filename, "wb")) == NULL) {
		LOG_E("can't open %s\n: %s", filename, strerror(errno));
		ret = -1;
		goto err_no;
	}

	//setup error handler, quite awful.
	struct my_error_mgr jerr;
	bzero(&jerr, sizeof(struct my_error_mgr));
	cinfo.err = jpeg_std_error(&jerr.pub);
	jerr.pub.error_exit = my_error_exit;
	if (setjmp(jerr.setjmp_buffer)) {
		ret = -1;
		//本来是打算对start_compress也做一下finish处理的，但想到这个程序已经出错了，
		//这时再做这个不知会否引起崩溃，所以没加
		if (create_compress == 1)
			goto err_create_compress;
		goto err_file;
	}
	
	jpeg_create_compress(&cinfo);
	create_compress = 1;
	jpeg_stdio_dest(&cinfo, outfile);
	cinfo.image_width = image_width; 	/* image width and height, in pixels */
	cinfo.image_height = image_height;
	cinfo.input_components = 3;		/* # of color components per pixel */
	cinfo.in_color_space = JCS_RGB; 	/* colorspace of input image */
	jpeg_set_defaults(&cinfo);
	jpeg_set_quality(&cinfo, quality, TRUE /* limit to baseline-JPEG values */);

	jpeg_start_compress(&cinfo, TRUE);
	
	row_stride = image_width * 3;	/* JSAMPLEs per row in image_buffer */

	while (cinfo.next_scanline < cinfo.image_height) {
		row_pointer[0] = & image_buffer[cinfo.next_scanline * row_stride];
		(void) jpeg_write_scanlines(&cinfo, row_pointer, 1);
	}

	jpeg_finish_compress(&cinfo);
	jpeg_destroy_compress(&cinfo);
	fclose(outfile);
	return 0;
err_create_compress:
	jpeg_destroy_compress(&cinfo);
err_file:  
	fclose(outfile);
err_no:
	return ret;
}

int seek_frame(AVFormatContext *pFormatCtx, int videoStream, int64_t time_s)
{
	int64_t seek_time = 0;
	if (videoStream >=0) {
		AVStream* stream = pFormatCtx->streams[videoStream];
		seek_time = (time_s*(stream->time_base.den))/(stream->time_base.num);
	}
	else seek_time = time_s * AV_TIME_BASE;
	int	ret = av_seek_frame(pFormatCtx, videoStream, seek_time, AVSEEK_FLAG_BACKWARD); 
	if (ret < 0) {
		fprintf(stderr, "error\n");
		return -1;
	}
	return 0;
}

#if 0
/**
 * 将AVFrame(YUV420格式)保存为JPEG格式的图片
 *
 * @param width YUV420的宽
 * @param height YUV42的高
 *
 */
int MyWriteJPEG(AVFrame* pFrame, int width, int height, int iIndex)
{
    // 输出文件路径
    char out_file[256] = {0};
    //sprintf_s(out_file, sizeof(out_file), "%s%d.jpg", "E:/temp/", iIndex);
	sprintf(out_file, "out_%d.jpg", iIndex);
	
    // 分配AVFormatContext对象
    AVFormatContext* pFormatCtx = avformat_alloc_context();
    
    // 设置输出文件格式
    pFormatCtx->oformat = av_guess_format("mjpeg", NULL, NULL);
    // 创建并初始化一个和该url相关的AVIOContext
    if( avio_open(&pFormatCtx->pb, out_file, AVIO_FLAG_READ_WRITE) < 0) {
        printf("Couldn't open output file.");
        return -1;
    }
    
    // 构建一个新stream
    AVStream* pAVStream = avformat_new_stream(pFormatCtx, 0);
    if( pAVStream == NULL ) {
        return -1;
    }
    
    // 设置该stream的信息
    AVCodecContext* pCodecCtx = pAVStream->codec;
    
    pCodecCtx->codec_id = pFormatCtx->oformat->video_codec;
    pCodecCtx->codec_type = AVMEDIA_TYPE_VIDEO;
    pCodecCtx->pix_fmt = PIX_FMT_YUVJ420P;
    pCodecCtx->width = width;
    pCodecCtx->height = height;
    pCodecCtx->time_base.num = 1;
    pCodecCtx->time_base.den = 25;
    
    // Begin Output some information
    av_dump_format(pFormatCtx, 0, out_file, 1);
    // End Output some information
    
    // 查找解码器
    AVCodec* pCodec = avcodec_find_encoder(pCodecCtx->codec_id);
    if( !pCodec ) {
        printf("Codec not found.");
        return -1;
    }
    // 设置pCodecCtx的解码器为pCodec
    if( avcodec_open2(pCodecCtx, pCodec, NULL) < 0 ) {
        printf("Could not open codec.");
        return -1;
    }
    
    //Write Header
    avformat_write_header(pFormatCtx, NULL);
    
    int y_size = pCodecCtx->width * pCodecCtx->height;
    
    //Encode
    // 给AVPacket分配足够大的空间
    AVPacket pkt;
    av_new_packet(&pkt, y_size * 3);
    
    // 
    int got_picture = 0;
    int ret = avcodec_encode_video2(pCodecCtx, &pkt, pFrame, &got_picture);
    if( ret < 0 ) {
        printf("Encode Error.\n");
        return -1;
    }
    if( got_picture == 1 ) {
        //pkt.stream_index = pAVStream->index;
        ret = av_write_frame(pFormatCtx, &pkt);
    }

    av_free_packet(&pkt);

    //Write Trailer
    av_write_trailer(pFormatCtx);

    //printf("Encode Successful.\n");

    if( pAVStream ) {
        avcodec_close(pAVStream->codec);
    }
    avio_close(pFormatCtx->pb);
    avformat_free_context(pFormatCtx);
    
    return 0;
}
#endif
static int do_media_snapshot(AVFormatContext *pFormatCtx, AVCodecContext *pCodecCtx, 
				int videoStream, OptionDmContext *dm_context)
{
	AVFrame         *pFrame;
	AVFrame         *pFrameRGB;
	AVPacket        packet;
	int             frameFinished;
	int             numBytes;
	uint8_t         *buffer;
	int ret = 0;
	pFrame = avcodec_alloc_frame();
	if( pFrame == NULL ) {
		LOG_E("avcodec_alloc_frame failed\n");
		ret = -1;
		goto err_no;
	}

	pFrameRGB = avcodec_alloc_frame();
	if( pFrameRGB == NULL ) {
		LOG_E("avcodec_alloc_frame failed\n");
		ret = -1;
		goto err_Frame;
	}
	numBytes = avpicture_get_size(PIX_FMT_RGB24, pCodecCtx->width,
			pCodecCtx->height);
	buffer = av_malloc(numBytes);
	if( buffer == NULL ) {
		LOG_E("av_malloc failed\n");
		ret = -1;
		goto err_FrameRGB;
	}
 
 	avpicture_fill( (AVPicture *)pFrameRGB, buffer, PIX_FMT_RGB24,
	pCodecCtx->width, pCodecCtx->height);
	seek_frame(pFormatCtx, videoStream, atoll(dm_context->seek_time));
	int i = 0;
	while( av_read_frame(pFormatCtx, &packet) >= 0 ) {
		if( packet.stream_index == videoStream ) {
			avcodec_decode_video2(pCodecCtx, pFrame, &frameFinished, &packet);

			if( frameFinished ) {
				struct SwsContext *img_convert_ctx = NULL;
				img_convert_ctx = 
					sws_getCachedContext(img_convert_ctx, pCodecCtx->width,
							pCodecCtx->height, pCodecCtx->pix_fmt,
							pCodecCtx->width, pCodecCtx->height,
							PIX_FMT_RGB24, SWS_BICUBIC,
							NULL, NULL, NULL);
				if( !img_convert_ctx ) {
					ret = -1;
					LOG_E("Cannot initialize sws conversion context\n");
					av_free_packet(&packet);
					goto err_buffer;
				}
				sws_scale(img_convert_ctx, (const uint8_t* const*)pFrame->data,
						pFrame->linesize, 0, pCodecCtx->height, pFrameRGB->data,
						pFrameRGB->linesize);
				if( i++ < 1 )
					test_jpeg(i, pCodecCtx->width, pCodecCtx->height,pFrameRGB->data[0],60);
					//MyWriteJPEG(pFrame, pCodecCtx->width, pCodecCtx->height, i);
					//SaveFrame(pFrameRGB, pCodecCtx->width, pCodecCtx->height, i);
				else
					break;
			}
		}
		av_free_packet(&packet);
	}
	av_free(buffer);
	av_free(pFrameRGB);
	av_free(pFrame);
	return 0;
err_buffer:
	av_free(buffer);
err_FrameRGB:
	av_free(pFrameRGB);
err_Frame:
	av_free(pFrame);
err_no:
	return ret;
}

int media_snapshot(OptionDmContext *dm_context)
{
	AVFormatContext *pFormatCtx = NULL;
	int             i, videoStream;
	AVCodecContext  *pCodecCtx;
	AVCodec         *pCodec;
	int ret = 0;
	if (!dm_context || !dm_context->input_file_name || !dm_context->output_file_name) {
		LOG_E("a argument is null!\ndm_context(%p)  input_file_name(%p)  output_file_name(%p)\n",
					dm_context, dm_context->input_file_name, dm_context->output_file_name);
		ret = -1;
		goto err_no;
	}
	av_register_all();	
	if( avformat_open_input(&pFormatCtx, dm_context->input_file_name, NULL, NULL) != 0 ) {
		ret = -1;
		goto err_no;
	}
	if( avformat_find_stream_info(pFormatCtx, NULL ) < 0 ) {
		ret = -1;
		goto err_format;
	}
	//av_dump_format(pFormatCtx, -1, dm_context->input_file_name, 0);
	videoStream = -1;
	for( i = 0; i < pFormatCtx->nb_streams; i++ )
		if( pFormatCtx->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO) {
			videoStream = i;
			break;
		}
	if( videoStream == -1 ) {
		ret = -1;
		goto err_format;
	}
	pCodecCtx = pFormatCtx->streams[videoStream]->codec;
	pCodec = avcodec_find_decoder(pCodecCtx->codec_id);
	if( pCodec == NULL ) {
		ret = -1;
		goto err_format;
	}

	if( avcodec_open2(pCodecCtx, pCodec, NULL) < 0 ){
		ret = -1;
		goto err_codec;
	}

	if (do_media_snapshot(pFormatCtx, pCodecCtx, videoStream, dm_context) < 0) {
		ret = -1;
		goto err_codec;
	}
	
	avcodec_close(pCodecCtx);
	avformat_close_input(&pFormatCtx);

	return 0;
err_codec:
	avcodec_close(pCodecCtx);	
err_format:
	avformat_close_input(&pFormatCtx);
err_no:
	return ret;
}
