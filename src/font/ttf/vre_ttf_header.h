
typedef struct ttf_offset_table
{
    vre_fix16   version;
    vre_uint16  num_tables;
    vre_uint16  search_range;   // (Maximum power of 2 <= numTables) x 16
    vre_uint16  entry_selector; // Log2(maximum power of 2 <= numTables)
    vre_uint16  range_shift;    // NumTables x 16-searchRange

} ttf_offset_table;

typedef struct ttf_table_directory
{
    vre_uint32  tag;      // 4 - byte identifier
    vre_uint32  offset;   // offset from beg. of TTF file.
    vre_uint32  checksum; // checksum for this table
    vre_uint32  length;
} ttf_table_directory;

/*
ULONG
CalcTableChecksum(ULONG *Table, ULONG Length)
{
ULONG Sum = 0L;
ULONG *Endptr = Table+((Length+3) & ~3) / sizeof(ULONG);

while (Table < EndPtr)
	Sum += *Table++;
return Sum;
}
*/

typedef struct ttf_header_table
{
    vre_fix16   table_version;
    vre_fix16   font_version;
    vre_uint32  checksum_adjustment;
    vre_uint32  magic;
    vre_uint16  flags;
    vre_uint16  units_per_em;
    vre_uint32  date_created;
    vre_uint32  date_modified;
    vre_int16   x_min;
    vre_int16   y_min;
    vre_int16   x_max;
    vre_int16   y_max;
    vre_uint16  mac_style;
    vre_uint16  lowest_rec_PPEM;
    vre_int16   font_direction_hint;
    vre_int16   index_to_loc_format;
    vre_int16   glyph_data_format;    
    
} ttf_header_table;


typedef struct ttf_location_table
{
    void    *pOffsets;
    
} ttf_location_table;


typedef struct ttf_glyph_table
{
    vre_int16   num_contours; 
    // If the number of contours is greater than or equal to zero, 
    // this is a simple glyph; if negative, this is a composite glyph.
    vre_int16   x_min;
    vre_int16   y_min;
    vre_int16   x_max;
    vre_int16   y_max;

} ttf_glyph_table;

// Glyph flags
#define GLYPH_FLG_ON_CURVE  1
#define GLYPH_FLG_X_SHORT   2
#define GLYPH_FLG_Y_SHORT   4
#define GLYPH_FLG_REPEAT    8
#define GLYPH_FLG_X_SAME   16
#define GLYPH_FLG_Y_SAME   32

typedef struct ttf_glyph_simple
{
    vre_uint16  end_points;
    vre_uint16  instruction_length;
    vre_uint8   *pInstructions;
    vre_uint8   *pFlags;
    void        *pXCoordinates;
    void        *pYCoordinates;   
} ttf_glyph_simple;

/*
do {
	USHORT flags;
	USHORT glyphIndex;
	if ( flags & ARG_1_AND_2_ARE_WORDS) {
	(SHORT or FWord) argument1;
	(SHORT or FWord) argument2;
	} else {
		USHORT arg1and2; // (arg1 << 8) | arg2 
	}
	if ( flags & WE_HAVE_A_SCALE ) {
		F2Dot14  scale;    // Format 2.14 
	} else if ( flags & WE_HAVE_AN_X_AND_Y_SCALE ) {
		F2Dot14  xscale;    // Format 2.14 
		F2Dot14  yscale;    // Format 2.14 
	} else if ( flags & WE_HAVE_A_TWO_BY_TWO ) {
		F2Dot14  xscale;    // Format 2.14 
		F2Dot14  scale01;   // Format 2.14 
		F2Dot14  scale10;   // Format 2.14 
		F2Dot14  yscale;    // Format 2.14 
	}
} while ( flags & MORE_COMPONENTS ) 
if (flags & WE_HAVE_INSTR){
	USHORT numInstr
	BYTE instr[numInstr]
	
*/

typedef struct ttf_glyph_composite
{
    vre_int16   flags;
    vre_int16   glyph_index;
    void        *pArguments;
} ttf_glyph_composite;


