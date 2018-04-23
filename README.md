# File-System
ext2-like File System by Efimochkin Sergey.

In runtime whole file system is also stored in dynamic memory.

Number of files is pre-set as NUMBER_OF_INODES.

NUMBER_OF_CHARS_IN_INDEX must be less than NUMBER_OF_BYTES_IN_BLOCK;  

NUMBER_OF_BYTES_IN_BLOCK / NUMBER_OF_CHARS_IN_INDEX preferably should be more than two, or there's practically no point 
in double addressing.


##Supported commands list

ls to list files in current directory

cd $name$ to go to directory named $name$

mkdir $name$ to create directory named $name$

touch $name$ $file text$ to create a file named $name$ with text of file $file text$

rmdir $name$ to delete directory named $name$ and all files in it

rm $name$ to delete file named $name$ from directory

read $name$ to print text of file named $name$

import $inner name$ $outer name$ to import file named $outer name$ from computer's file system and save it 

  as file named $inner name$ in this file system
  
export $inner name$ $outer name$ to export file named $inner name$ into computer's file system as a file 

  named $outername$
  
save to save all changes made in the filesystem

exit to save and exit

help to see help message
