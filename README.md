# A archive format I implemented in both C# and C++

## Why?

Boredom

## Things I learned

- I suck at C++
- I LOVE C# (it's really fun)

## Notes

- The archive file format is really basic, see diagram below
- The C++ implementation is buggy at best, broken at worst

## How the archive is structured

```
At the beginning of the archive:
+-------------------+---------------------------------------------------------------+--------------+--------------+ - - -
|                   |                                                               |              |              |
|    HEADER_SIZE    |                          HEADER                               |    File 1    |    File 2    |    File N
|      (int32)      |    (flattened array of file names concatenated using NULL)    |              |              |
+-------------------+---------------------------------------------------------------+--------------+--------------+ - - -

The file-block looks like this:
+-----------------+---------------------+
|                 |                     |
|    FILE_SIZE    |    RAW_FILE_DATA    |
|     (int32)     |                     |
+-----------------+---------------------+
```