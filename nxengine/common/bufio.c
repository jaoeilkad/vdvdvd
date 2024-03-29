
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "bufio.fdh"

uint8_t read_U8(const uint8_t **data, const uint8_t *data_end)
{
	if (*data > data_end)
		return 0xfe;
	return *(*data)++;
}

uint16_t read_U16(const uint8_t **data, const uint8_t *data_end)
{
	const uint8_t *ptr = *data;

	if ((ptr + 1) > data_end)
		return 0xfefe;

	*data = (ptr + 2);
	// we should not just cast to a uint16_t, as some processors
	// e.g. ARM would have alignment issues then, plus endian issues on others.
	return (ptr[1] << 8) | ptr[0];
}

uint32_t read_U24(const uint8_t **data, const uint8_t *data_end)
{
	uint32_t a = read_U16(data, data_end);
	uint32_t b = read_U8(data, data_end);

	return a | (b << 16);
}

uint32_t read_U32(const uint8_t **data, const uint8_t *data_end)
{
	const uint8_t *ptr = *data;
	
	if ((ptr + 3) > data_end)
		return 0xfefefefe;
	
	*data = (ptr + 4);
	return (ptr[3] << 24) | (ptr[2] << 16) | (ptr[1] << 8) | ptr[0];
}
