# Spelling Checker
This program checks for any spelling errors in a given file using a given dictionary.  
Dictionary is created in the memory as a map. Each entry in the map has an arraylist of words starting with the same letter that used as the key for the entry.  
Words in the arraylist are ordered alphabetically at first. During the spell checking process, words are reordered according to their total access (hit) counts in realtime.

### To compile:
gcc spell_checker.c -o spell_checker

### Usage:
spell_checker <dictionary_file> <file_to_check>

### Sample input:
./spell_checker sampleDictionary.txt sampleFile.txt

### Output:
  
DICTIONARY BEFORE SORTING:  
apple - aid - art - a - ah  
banana - bey  
carrot - chimpanze - chat - cey  
durian  
eh  
goose  
his - her - his - hers - heh  
left - look - line - lion  
meh  
tiger - th - te - tah - tea - tee - ten - tex - teh  
  
DICTIONARY AFTER SORTING:  
a - ah - aid - apple - art  
banana - bey  
carrot - cey - chat - chimpanze  
durian  
eh  
goose  
heh - her - hers - his - his  
left - line - lion - look  
meh  
tah - te - tea - tee - teh - ten - tex - th - tiger  
  
TEST RESULTS:  
Incorrect word detected at 10. word in the file: tigger  
Incorrect word detected at 16. word in the file: tin  
File check is completed.  
  
UPDATED VERSION OF DICTIONARY (Words are sorted realtime according to their hit counts):  
art - a - ah - aid - apple  
banana - bey  
chimpanze - carrot - cey - chat  
durian  
eh  
goose  
heh - her - hers - his - his  
lion - left - line - look  
meh  
tah - te - tea - tee - teh - ten - tex - th - tiger  