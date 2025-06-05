
Player
|Start|Free (3 bits)
00 00 00 00
6 bits 

Start
 | Tile ID |
 00 00 00 00
 2|4 8|16 32|64 128|256

20x10 = 200 8x8 total tiles
10x5 = 50 (16x16)
flip 2 bits

20 columns

|Platform|flip <- 00 () 10 (h) 01 (v) 11(hv)
 00 00 00 00 
 2|4 8|16 32|64 128|256

15 section = 15 bytes

|enemy|flip|
 00 00 00    00 
 
15 sections = 6 * 15 = 90/8 = 12(11.25)
10 2 tile sections (4) = 4*10 = 40 bits = 40/8 = 5 bytes

Total enemy bytes = 17*

15  sections (1 byte each)
17* enemies  (1 byte each)
1   Player   (1* byte)
1   Start    (1 byte)

Level Code is 34 chars (bytes)


14 byte mask patterns (16 chars)
23 byte patters       (32 chars)


34 chars, 16 char selection, can we make a 32 char selection and reduce the 34 chars code