## reporting bugs
open the issues tab and create an issue with the bug template. make sure that you are descriptive and specific.

images and/or sample code is much appreciated, and goes a long way to fixing it.

in general, follow the template and you'll be fine.

## suggesting features
New features should solve a problem that is both common and unique. A new feature won't be added simply because you can't be bothered to write it yourself, and likewise it is unlikely to be added if the problem it solves is too niche.

you should be ready to justify why your feature should be added.

## opening pull requests
Pull requests should be atomic, meaning they only do one thing at a time. Some leeway is given, but this guideline exists to make identifying bugs and version control easier. when in doubt, just open a second pull request.

Your pull request must be tested before being merged.

## style guidelines
code should be written in C++, and files given the extension of .cpp and .hpp

Mercury code submitted is to be given the extension of .mrc

Code should be easily readable, and should be clear what it does just by looking at it. This means descriptive variable and function names, and sensibly laid out code.

inline assembly or other such micro-optimizations are not likely to be accepted unless they are proven to dramatically improve performance.

Though written in C++, mercury takes a C coding style, and using C++ features like classes or std:: is heavily discouraged. You should do your best to find a different implementation, and such code is only to be accepted where absolutely necessary.
nullptr should be used over NULL, and malloc() over new.

names of functions, types, and constants that exist for mercury, and that are included in header files should be given names that denote they belong to mercury, such as prefixing with mercury_ or M_. This makes code more readable, and reduces the change of name clashes.
