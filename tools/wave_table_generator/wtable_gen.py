import math
import os

source_file = 'wave_tables.c'
header_file = 'wave_tables.h'

table_size = 2048
table_width = 16
table_amp = 2047

def triangle(x):
   x_wrap = x % (2*math.pi)
   res = 0
   if x_wrap <= math.pi/2:
      res = 2*x_wrap / math.pi
   elif x_wrap <= 3*math.pi/2:
      res = 2 - (2*x_wrap/math.pi)
   elif x_wrap <= 2*math.pi:
      res = (x_wrap - 2*math.pi)*(2/math.pi)
   return res

def saw1(x):
   x_wrap = x % (2*math.pi)
   res = (x_wrap/math.pi) - 1
   return res

def saw2(x):
   x_wrap = x % (2*math.pi)
   res = 1 - (x_wrap/math.pi)
   return res

def square(x):
   x_wrap = x % (2*math.pi)
   res = 0
   if x_wrap <= math.pi:
      res = 1
   else:
      res = -1
   return res

def format_print(file, x):
   if sine_out < 0:
      if sine_out > -10:
         file.write('%u,    ' % x)
      elif sine_out > -100:
         file.write('%u,   ' % x)
      elif sine_out > -1000:
         file.write('%u,  ' % x)
      else:
         file.write('%u, ' % x)
   else:
      if sine_out < 10:
         file.write('%u,     ' % x)
      elif sine_out < 100:
         file.write('%u,    ' % x)
      elif sine_out < 1000:
         file.write('%u,   ' % x)
      else:
         file.write('%u,  ' % x)

with open(header_file, 'w') as out_file:
   out_file.write('#ifndef __WAVE_TABLES_H__\n#define __WAVE_TABLES_H__\n\n')
   out_file.write('#include <stdlib.h>\n#include <stdint.h>\n\n')

   out_file.write('#define WAVE_TABLE_SIZE %s\n\n' % table_size)

   out_file.write('extern const int16_t sine_table[];\n')
   out_file.write('extern const int16_t triang_table[];\n')
   out_file.write('extern const int16_t saw1_table[];\n')
   out_file.write('extern const int16_t saw2_table[];\n')
   out_file.write('extern const int16_t square_table[];\n\n')

   out_file.write('#endif')

with open(source_file, 'w') as out_file:

   out_file.write('#include "%s"\n' % header_file)

   out_file.write('\n\nconst int16_t sine_table[WAVE_TABLE_SIZE] =\n{\n\t')

   for i in range(table_size-1):
      if i % table_width == 0 and i != 0:
         out_file.write('\n\t')
      sine_out = int(table_amp * math.sin(i * (2*math.pi/table_size)))
      
      format_print(out_file, sine_out)
   
   out_file.write('%u\n};' % int(table_amp * math.sin((table_size-1) * (2*math.pi/table_size))))

   out_file.write('\n\nconst int16_t triang_table[WAVE_TABLE_SIZE] =\n{\n\t')

   for i in range(table_size-1):
      if i % table_width == 0 and i != 0:
         out_file.write('\n\t')
      sine_out = int(table_amp * triangle(i * (2*math.pi/table_size)))
      
      format_print(out_file, sine_out)
   
   out_file.write('%u\n};' % int(table_amp * triangle((table_size-1) * (2*math.pi/table_size))))

   
   
   out_file.write('\n\nconst int16_t saw1_table[WAVE_TABLE_SIZE] =\n{\n\t')

   for i in range(table_size-1):
      if i % table_width == 0 and i != 0:
         out_file.write('\n\t')
      sine_out = int(table_amp * saw1(i * (2*math.pi/table_size)))
      
      format_print(out_file, sine_out)
   
   out_file.write('%u\n};' % int(table_amp * saw1((table_size-1) * (2*math.pi/table_size))))



   out_file.write('\n\nconst int16_t saw2_table[WAVE_TABLE_SIZE] =\n{\n\t')

   for i in range(table_size-1):
      if i % table_width == 0 and i != 0:
         out_file.write('\n\t')
      sine_out = int(table_amp * saw2(i * (2*math.pi/table_size)))
      
      format_print(out_file, sine_out)
   
   out_file.write('%u\n};' % int(table_amp * saw2((table_size-1) * (2*math.pi/table_size))))



   out_file.write('\n\nconst int16_t square_table[WAVE_TABLE_SIZE] =\n{\n\t')

   for i in range(table_size-1):
      if i % table_width == 0 and i != 0:
         out_file.write('\n\t')
      sine_out = int(table_amp * square(i * (2*math.pi/table_size)))
      
      format_print(out_file, sine_out)
   
   out_file.write('%u\n};' % int(table_amp * square((table_size-1) * (2*math.pi/table_size))))
 
os.system('cp %s ../../core/src/' % source_file)
os.system('cp %s ../../core/inc/' % header_file)