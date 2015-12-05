Hertz :watch: <a href="https://travis-ci.org/r-lyeh/hertz"><img src="https://api.travis-ci.org/r-lyeh/hertz.svg?branch=master" align="right" /></a>
====

- Hertz is a simple framerate locker (C++11)
- Hertz features auto-frameskip.
- Hertz features dynamic/variable framerate locking.
- Hertz is tiny (~75 LOC), cross-platform, self-contained and header-only.
- Hertz is zlib/libpng licensed.

### API
```c++
double /*fps*/ hertz::lock( 60 /*Hz*/, []{/*logic*/}, []{/*render*/} );
          void hertz::unlock(); // quit
```

### Changelog
- v1.0.0 (2015/12/05): Add unlock function
- v1.0.0 (2015/09/19): Initial commit
