/*
** License Applicability. Except to the extent portions of this file are
** made subject to an alternative license as permitted in the SGI Free
** Software License B, Version 1.1 (the "License"), the contents of this
** file are subject only to the provisions of the License. You may not use
** this file except in compliance with the License. You may obtain a copy
** of the License at Silicon Graphics, Inc., attn: Legal Services, 1600
** Amphitheatre Parkway, Mountain View, CA 94043-1351, or at:
**
** http://oss.sgi.com/projects/FreeB
**
** Note that, as provided in the License, the Software is distributed on an
** "AS IS" basis, with ALL EXPRESS AND IMPLIED WARRANTIES AND CONDITIONS
** DISCLAIMED, INCLUDING, WITHOUT LIMITATION, ANY IMPLIED WARRANTIES AND
** CONDITIONS OF MERCHANTABILITY, SATISFACTORY QUALITY, FITNESS FOR A
** PARTICULAR PURPOSE, AND NON-INFRINGEMENT.
**
** Original Code. The Original Code is: OpenGL Sample Implementation,
** Version 1.2.1, released January 26, 2000, developed by Silicon Graphics,
** Inc. The Original Code is Copyright (c) 1991-2000 Silicon Graphics, Inc.
** Copyright in any portions created by third parties is as indicated
** elsewhere herein. All Rights Reserved.
**
** Additional Notice Provisions: The application programming interfaces
** established by SGI in conjunction with the Original Code are The
** OpenGL(R) Graphics System: A Specification (Version 1.2.1), released
** April 1, 1999; The OpenGL(R) Graphics System Utility Library (Version
** 1.3), released November 4, 1998; and OpenGL(R) Graphics with the X
** Window System(R) (Version 1.3), released October 19, 1998. This software
** was created using the OpenGL(R) version 1.2.1 Sample Implementation
** published by SGI, but has not been independently verified as being
** compliant with the OpenGL(R) version 1.2.1 Specification.
*/

/*
 * flistsorter.c++
 *
 * $Date: 2001/03/17 00:25:40 $ $Revision: 1.1 $
 * $Header: /cvsroot/mesa3d/Mesa/si-glu/libnurbs/internals/flistsorter.cc,v 1.1 2001/03/17 00:25:40 brianp Exp $
 */

#include "glimports.h"
#include "flistsorter.h"

FlistSorter::FlistSorter( void ) : Sorter( sizeof( REAL ) )
{
}

void
FlistSorter::qsort( REAL *p, int n )
{
    Sorter::qsort( (char *)p, n );
}

int
FlistSorter::qscmp( char *i, char *j )
{
    REAL f0 = *(REAL *)i;
    REAL f1 = *(REAL *)j;
    return (f0 < f1) ? -1 : 1;
}

void
FlistSorter::qsexc( char *i, char *j )
{
    REAL *f0 = (REAL *)i;
    REAL *f1 = (REAL *)j;
    REAL tmp = *f0;
    *f0 = *f1;
    *f1 = tmp;
}

void
FlistSorter::qstexc( char *i, char *j, char *k )
{
    REAL *f0 = (REAL *)i;
    REAL *f1 = (REAL *)j;
    REAL *f2 = (REAL *)k;
    REAL tmp = *f0;
    *f0 = *f2;
    *f2 = *f1;
    *f1 = tmp;
}
