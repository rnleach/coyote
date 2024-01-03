# coyote

A very basic platform library.

Coyotes are extremely adaptable animals. They've been in the Continental U.S. since before humans, and they've remained even
when many other animals have been pushed out by urbanization. There are reports of coyotes living in Los Angeles! Since a
platform library is about adapting to different environments, I thought I'd name mine after coyotes, since they are so 
adaptable!

Goals & Non-goals:
 1. I only implement the things I need. If it's in here, I needed it at some point.

 2. Single header library. Keep it simple to use and include in a program.

 3. NOT threadsafe, though it MAY implement types and functions for abstracting over OS facilities for thread 
    synchronization.

 4. NO global mutable state and reentrant functions. I'll do the best I can here, and if I fail I'll document it, but at
    some level I'm at the mercy of the OS's I support.
 
 5. I plan to support Windows, Mac OSX (BSD unix), and Linux (Ubuntu). The last two may just be considered POSIX for now, 
    but I'm not commited to Posix.

## Releases

### Version 1.1.0 - IN PROGRESS
  - (XXXX-XX-XX)
  - Bug fix for file iterator.
  - Add more convenience functions for file IO.
  - Create a build program to combine the files into a single header instead of relying on includes.

### Version 1.0.0
  - (2023-10-21) Initial release.
  - Get the current system time.
  - Basic file IO.
  - Read only memory mapped files.
  - Allocate & free large chunks of memory.

