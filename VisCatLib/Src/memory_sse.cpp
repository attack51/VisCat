#include "stdafx.h"
#include "memory_sse.h"

void memcpy_sse(void* dest, const void* source, size_t total_length)
{
	// copy = (head normal copy) + (sse copy) + (foot normal copy)
	// head_bytes = (16 - (dest % 16)) % 16
	// sse copy loop (*16 = bytes) = (total_length - head_bytes)/16
	// foot_bytes = (total_length - head_bytes)%16

	size_t head_bytes = (16 - (((size_t)(dest)) & 0xf)) & 0xf;
	if (head_bytes > total_length) head_bytes = total_length;

	size_t non_head_bytes = total_length - head_bytes;

	__asm
	{
;COPY_HEAD:
		mov esi, source
		mov edi, dest

		mov ecx, head_bytes
		rep movsb
		
;COPY_SSE2:
		mov ecx, non_head_bytes
		shr ecx, 4
		cmp ecx, 0
		jz COPY_FOOT

COPY_SSE2_LOOP:
		movdqu xmm0, [esi]
		movntdq xmmword ptr [edi], xmm0
		
		add esi, 16
		add edi, 16

		loop COPY_SSE2_LOOP

COPY_FOOT:
		;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
		; if no foot bytes  then go to sse copy
		mov ecx, non_head_bytes
		and ecx, 15
		rep movsb
	}
}