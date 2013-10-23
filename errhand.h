/********************** Start of ERRHAND.H **********************/

#ifndef _ERRHAND_H
#define _ERRHAND_H

#ifdef ___STDC___
void fatal_error( char *fmt, ... );

#else /* ___STDC___ */
void fatal_error();

#endif /* ___STDC___ */
#endif /* _ERRHAND_H */

/********************** End of ERRHAND.H *************************/

