
#include <imageloader.h>

#include "context.h"

#include <stdio.h>
#include <stdarg.h>

void print_to_log(ImgloadContext ctx, ImgloadLogLevel level, const char* format, ...)
{
	if (!ctx->log.handler)
	{
		return;
	}

	char buffer[1024];
	va_list args;
	va_start(args, format);
	vsnprintf(buffer, sizeof(buffer) / sizeof(buffer[0]), format, args);

	ctx->log.handler(ctx->log.ud, level, buffer);

	va_end(args);
}