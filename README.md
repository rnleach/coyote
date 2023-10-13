# coyote

A very basic platform library.

Coyotes are extremely adaptable animals. They've been in the Continental U.S. since before humans, and they've remained even
when many other animals have been pushed out by urbanization. There are reports of coyotes living in Los Angeles! Since a
platform library is about adapting to different environments, I thought I'd name mine after coyotes, since they are so 
adaptable!

Goals & Non-goals:
 1. I only implement the things I need. If it's in here, I needed it at some point.

 2. Single header (maybe + single C) file. Keep it simple to use and included in a program.

 3. NOT threadsafe, though it may implement types and functions for abstracting over OS facilities for thread 
    synchronization.

 4. NO global mutable state and reentrant functions. I'll do the best I can here, and if I fail I'll document it, but at
    some level I'm at the mercy of the OS's I support.
 
 5. I plan to support Windows, Mac OSX (BSD unix), and Linux (Ubuntu). The last two may just be considered Posix, but I'm
    not commited to Posix.

## Design Notes

    None yet.

## Releases

### Version 1.0.0 - IN PROGRESS
  - (YYYY-MM-DD) Initial release.

