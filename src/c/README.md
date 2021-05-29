This directory's source code is *amalgamated*, similar to
[SQLite](https://www.sqlite.org/amalgamation.html), to form a [single file C
library](https://github.com/nothings/stb/blob/master/docs/stb_howto.txt).

After editing any source code, run `script/gen-release-c.go` from the
repository's top-level directory to produce the single `release/c/etc.c` file
used by the example programs.

During local development (but not for committing), an alternative approach
avoids the need to run that script. After editing any source code, also modify
the example programs to change lines like:

```
#include "../../release/c/etc.c"
```

to instead be:

```
#include "../../src/c/aaa_package.h"
```
