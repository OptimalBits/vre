
from xml.parsers.expat import *
import re
from array import *
import os.path

groups = list ()


###
#
#  vre binary format
#  One or more chunks. Every chunk:  
#
#  chunk id   ( 4 bytes )
#  chunk size ( 4 bytes ), not including id and size fields.
#  chunk data ( n bytes of raw data, could be even more chunkds ).
#
#
# Available chunks:
# 'EOFC' -> End of chunk
# 'CGRP' -> Group chunk
#           Style:
#              'FILL' Fill colour
#              'FOPA' Fill Opacity
#              'STRK' Stroke colour
#              'STRW' Stroke width

# 'CPTH' -> Path chunk
# 'CSTL' > Style chunk
#
#
###

#
# Attributes 
#

#
# TODO: Use pyparsing or some more advance module to parse every type of attribute.
# SVG is a bit complex in its structure.
#

def parse_color ( input ):
    output = None
    if input[0]=='#':
        input = input[1:]
        if len(input) == 6:
            output = long ( input, 16 )
        elif len(input) == 3:
            s = input[0] + input[0] + input[1] + input[1] + input[2] + input[2]
            output = long ( s, 16 )
    
    return output
    
def parse_opacity ( input ):
    output = float ( input )
    if output > 1.0:
        output = 1.0
    if output < 0:
        output = 0
    return output

#
# TODO: implement unit support.
#    
def parse_length ( input ):
    return float ( input )

def parse_svg_attributes ( attrs ):
    svg_attrs = ['style']
    svg_attr_dict = dict()
    
    for key in attrs:
        value = attrs[key]
        if key in svg_attrs:
            value_list = value.strip().strip(';').split(';')
            attr_dict = dict()
           
            for a in value_list:
                k = a.split(':')[0].strip()
                v = a.split(':')[1].strip()
                
                if k in ['fill','stroke']:
                    v = parse_color ( v )
                elif k in ['stroke-width']:
                    v = parse_length ( v )
                elif k in ['fill-opacity', 'stroke-opacity']:
                    v = parse_opacity ( v )
                    
                attr_dict[k] = v

            svg_attr_dict[key] = attr_dict
            
    if len ( svg_attr_dict ) == 0:
        return None
    else:
        return svg_attr_dict
       
PAINT_ATTR_FILL = 1L
PAINT_ATTR_FILL_SOLID = 2L
PAINT_ATTR_FILL_GRADIENT = 4L
PAINT_ATTR_FILL_OPACITY = 8L
PAINT_ATTR_STROKE = 256L
PAINT_ATTR_STROKE_WIDTH = 512L
PAINT_ATTR_STROKE_OPACITY = 1024L

class paint_attributes:
    def __init__ ( self, attrs ):
        self.only_defaults = True
       
        svg_attributes = parse_svg_attributes ( attrs )
        
        if svg_attributes != None and svg_attributes.has_key ('style'):
            style_attrs =  svg_attributes['style']
            
            if style_attrs.has_key ( 'fill' ):
                self.fill = style_attrs['fill']
                self.only_defaults = False 
            else:
                self.fill = None
        
            if style_attrs.has_key ('fill-opacity'):
                self.fill_opacity = style_attrs['fill-opacity']
                self.only_defaults = False
            else:
                self.fill_opacity = 1.0
        
            if style_attrs.has_key ('stroke' ):
                self.stroke = style_attrs['stroke']
                self.only_defaults = False
            else:
                self.stroke = None
        
            if style_attrs.has_key ('stroke-width' ):
                self.stroke_width = style_attrs['stroke-width']
                self.only_defaults = False
            else:
                self.stroke_width = 1.0
        
    def serialize (self, data_array ):

        if not self.only_defaults:
            tmp_array = array ('l')
            data_array.fromstring('PNTA')
            avail_attrs = 0L
        
            if self.fill != None:
                tmp_array.append (self.fill)
                tmp_array.append (long(self.fill_opacity * 65536))
                avail_attrs = avail_attrs | PAINT_ATTR_FILL | PAINT_ATTR_FILL_OPACITY
                
            
            if self.stroke != None:
                tmp_array.append (self.stroke)
                tmp_array.append (long(self.stroke_width *65536))
                avail_attrs = avail_attrs | PAINT_ATTR_STROKE | PAINT_ATTR_STROKE_WIDTH
            
            data_array.append (len(tmp_array)*tmp_array.itemsize + 4) #length
            data_array.append ( avail_attrs )
            data_array.extend (tmp_array)
            
            print data_array

#
# Groups
#

class group(paint_attributes):
    def __init__ ( self, attrs ):
        
        self.primitives = list ()
        paint_attributes.__init__ ( self, attrs )
        
    def serialize ( self, data_array ):
        
        tmp_array = array ('l')
        data_array.fromstring ('CGRP')
        
        paint_attributes.serialize ( self, tmp_array )
          
        for primitive in self.primitives:
            primitive.serialize ( tmp_array )
        
        # length
        data_array.append (len(tmp_array)*tmp_array.itemsize) 
        data_array.extend (tmp_array)
        
    def add_primitive (self, primitive ):
        self.primitives.append ( primitive )
        
#
# Paths
#

def serialize_coordinates ( coordinates ):
    data_array = array ('l')
    fixed_point_coords = list ()
    
    for coord in coordinates:
        fixed_point_coords.append ( long (float (coord) * 65536) )
       
    data_array.extend ( fixed_point_coords )
    return data_array
    
class path(paint_attributes):

    def __init__ (self, attrs):
        
        paint_attributes.__init__ ( self, attrs )
        
        if attrs.has_key ( 'd' ):
            self.path_data = parse_path_data ( attrs['d'] )
        else:
            self.path_data = None
        
    def serialize ( self, data_array ):
        tmp_array = array ('l')
        data_array.fromstring('PATH')
        
        paint_attributes.serialize ( self, tmp_array )
        
        # Serialize all cmd as relatives and using fixed point 16.16
        # arithmetic.
        # TODO: Apply proper scale matrix in case values dont fit
        # in 16.16
        for cmd in self.path_data:
            coord_array = None
            if cmd[0] in 'MCL':
                coord_array = serialize_coordinates ( cmd[1] )
            elif cmd[0] == 'z':
                pass
    
            length = len ( cmd[1] ) / 2
            cmd_and_length = long (ord (cmd[0])) | ( length << 16 ) 
            tmp_array.append ( cmd_and_length )
            
            if coord_array != None:
                tmp_array.extend ( coord_array )

        # length
        data_array.append (len(tmp_array)*tmp_array.itemsize) 
        data_array.extend (tmp_array)

#
# Paths
#

re_cmd = "[MmZzLlHhVvCcSsQqTtAa]"
re_float = "-?[0-9]+(?:\.[0-9]+)?"

re_path = "\s*(?:(" + re_cmd + ")|(" + re_float + "))"

pattern = re.compile ( re_path )

def parse_path_data ( data ):
    path_data = list ()
  
    match = pattern.match ( data )
    if match.group (1) != None:
        current_cmd = match.group(1)
        coords = list ()
    else:
        pass # error
  
    num_coords = 0
    num_cmds = 0
  
    while match != None:
        match = pattern.match ( data, match.end() )
        
        if match != None:
            if match.group (1) != None:
                        num_coords = num_coords+ len ( coords )
                        num_cmds = num_cmds + 1
                    #if len ( coords ) > 0:
                        path_data.append ( (current_cmd, coords ))
                        current_cmd = match.group(1)
                        coords = list ()
         
                    #else:
                     #   pass
                
            elif match.group(2) != None:
                coords.append ( match.group(2) )
               
    path_data.append ( (current_cmd, coords ))

    return path_data        
   
#   
# Serialize as .vre
#

def save_vre ( filename, groups ):
    data_array = array('l')
    
    for g in groups:
        g.serialize ( data_array )
        
    data_array.fromstring ('CEOF')

    f = open (filename, 'wb')
    data_array.tofile (f)
    f.close ()

#
# Main Callback functions.
#

def start_element(name, attrs):
    global current_group
    
    # print 'Start element:', name, attrs
    if name == 'g':
        current_group = group ( attrs )
        groups.append ( current_group )
    
    elif name == 'path':
        current_group.add_primitive ( path ( attrs ))
      
def end_element(name):
    global current_group
    
    if name == 'g':
        current_group = None
    
def char_data(data):
    # print 'Character data:', repr(data)
    pass
    
def parseSVG ( filename ):
    f = open ( filename, 'r' )
   # svg_data = f.read ()
    
    parser = ParserCreate ()
    
    parser.StartElementHandler  = start_element
    parser.EndElementHandler    = end_element
    parser.CharacterDataHandler = char_data
    
    parser.ParseFile ( f )
    
    save_vre ( os.path.basename (filename)+'.vre', groups )    
    
        
parseSVG ( 'tiger.svg' )

