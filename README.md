# cg'BASE


# cg'BASE

> **g'BASE, the database management system**
>
> In today's environment, obtaining the information you want is not the problem - keeping track of it is! Finding that crucial piece of information, quickly and painlessly, is getting harder all the time. **g'BASE, the database management system** quickly and easily creates screens in any format to handle your simplest to most complex tasks.
>
> With 600 records per file, search through your data with as many fields as you'll ever need. Editing is simple and straightforward. So easy to use, no instruction manual is required!! Get what you need. when you need it and keep it organized! Use **g'BASE**.

I stumbled upon g'base while looking through old 5.25 inch diskettes I acquired and after loading it up, I was intrigued by how simple the software was. So much so that I probably would have used it back in the day.

**g'BASE, the database management system** was designed to be used with the C64 and C128. The diskette is labeled as part of the "HOME MANAGEMENT SERIES - LOAD'N'GO SOFTWARE" developed by Green Valley Publishing (A division of ShareData, Inc).

This repository is my attempt to recreate the software using C while trying to stay within the storage constraints that were available at the time [^1]. This is repository will not contain a dump of the original software nor will it be feature compatible. I also will not be using the original source to help build the C version. I am going to add new features and tweak the design a bit to work better with modern terminals. With that said, this software is not intended to be ran on the C64 even though there are ways to [cross compile C using llvm-mos](https://www.c64-wiki.com/wiki/llvm-mos). Maybe in the future I will attempt something like that :)

[^1]: I will try my best to fit the database within the size of the [5.25 diskette](https://www.c64-wiki.com/wiki/Diskette) which is about 500 bytes. I also would have _loved_ to design the application to only use the same memory that was available on the C64 but I feel that is beyond my skill set at this time.

## How to build & run

```bash
make
make run
```

## Features & Limitations

> [!Only Limitations]
> I'm currently in the process of figuring out how best to build the TUI. Once that is complete I'll start working on the actually application.
>
> Once I'm ready to start working on the application code, I'll update the features and note any limitations (most limitations will likely be a result of developer skill issues). I'll also document all the features of the original application and note where this application deviates.

## Why am I going this?

Mainly because I would like to improve my knowledge of C and system level programming. In my opinion, the best way to do this is by building stuff that is interesting. That thing doesn't have to be an original idea either. I believe that re-creating a piece of old software has the potential to teach me something about writing software since it will make me think more about how it was built and what constraints the developers faced while developing it.

I would also like to use this project to help me learn more about other languages like: c++, rust, and zig. I suppose this is my own to-do application of sorts :P

## Contributing
Since this is a more of a personal and experiential project, I won't be accepting pull requests. If you use this piece of software and discover a bug, feel free to create an issue with details on the issue.
