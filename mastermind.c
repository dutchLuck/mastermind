/*
 * M A S T E R M I N D . C
 *
 * C language code to play text mode mastermind
 *
 * mastermind.c last edited on Tue Apr 16 21:45:57 2024 
 * 
 */

/*
 * User choices; -
 *  choose the number of colors (code letters) will be used in the game (2 - 20)
 *  choose the color/letter code length (4 - 10)
 *  choose the maximum number of guesses the player has (7 - 20)
 *  choose whether or not colors/letters may be repeated in the code
 */

/*
 * A text version example:       1.   ADEF  |  XXO-
 *  Translates to : 1st Guess, User guessed ADEF and
 *  two correct code letters and in the correct places
 *  one correct code letter in the wrong place and
 *  one code letter incorrect.
 */

#include <stdio.h>  /* printf() getline() feof() */
#include <string.h> /* strspn() strlen() strchr() strcmp() strncpy() */
#include <ctype.h>  /* tolower() */
#include <stdlib.h> /* rand() srand() strtol() atexit() exit() */
#include <time.h>   /* clock_gettime() */
#include <signal.h>		/* signal() */

#define FALSE ((int) 0)
#define TRUE (! FALSE)
#define CORRECT_MARKER 'X'
#define INCORRECT_MARKER '-'
#define WRONG_POSITION_MARKER 'O'
#define ALREADY_SCORED '-'
#define MINIMUM_NUMBER_OF_CODE_SYMBOLS 2
#define DEFAULT_NUMBER_OF_CODE_SYMBOLS 7
#define MAXIMUM_NUMBER_OF_CODE_SYMBOLS 20
#define MINIMUM_CODE_LENGTH 4
#define MAXIMUM_CODE_LENGTH 10
#define MINIMUM_NUMBER_OF_GUESSES 7
#define MAXIMUM_NUMBER_OF_GUESSES 20
#define GET_LINE_MALLOC_SZ 16

int  playerSuccesses = 0;     /* games won by the player */
int  totalGamesPlayed = 0;
int  numberOfCodeSymbols = DEFAULT_NUMBER_OF_CODE_SYMBOLS;
int  codeWidth = MINIMUM_CODE_LENGTH;
int  totalNumberOfGuesses = MINIMUM_NUMBER_OF_GUESSES;
int  codeLettersCanBeRepeated = TRUE;
char  validCodeLetters[ MAXIMUM_NUMBER_OF_CODE_SYMBOLS + 1 ];      /* Array of allowed code letters in current guess + '\0' */
char *  playerEnteredLine = NULL;       /* For use by getLine() */
size_t linecap = 0;
char  resultSeparatorStrng[] = "  |  ";     /* separates guess from score when result for round is printed */


int  getLine( char **  linePtr, size_t *  lineSize, FILE *  fp )  {
    char *  chrPtr;
    size_t  chrCnt = 0;
    int  chr;

    if( *linePtr == NULL ) {
        if(( *linePtr = malloc( GET_LINE_MALLOC_SZ )) == NULL )  return( -1 );  /* Cannot recover */
        *lineSize = GET_LINE_MALLOC_SZ;
#ifdef DEBUG
        printf( "\nDEBUG: After malloc() Line buffer size is now %ld\n", *lineSize );
#endif
    }
    chrPtr = *linePtr;
    do {
        if(( chr = fgetc( fp )) == EOF )  return( -1 );
        else {
            if( ++chrCnt >= *lineSize ) {
                if(( *linePtr = realloc( *linePtr, *lineSize + GET_LINE_MALLOC_SZ )) == NULL )  return( -1 );  /* Cannot recover */
                *lineSize += GET_LINE_MALLOC_SZ;
#ifdef DEBUG
                printf( "\nDEBUG: After realloc() Line buffer size is now %ld\n", *lineSize );
#endif
            }
            *chrPtr++ = ( char ) chr;
            *chrPtr = '\0';
        }
    } while( chr != '\n' );
    return( chrCnt );
}


int  getYesOrNoResponseFromPlayer( char *  questionString, int  defaultResult )  {
    int  result = defaultResult;    /* default to No response */
    char *  enteredLineChrPtr;
    ssize_t lineLen;
    size_t  startIndexOfFirstNonSpaceChar;

    do  {
        printf( "%s: yes or no (defaults to %s) : ", questionString, defaultResult ? "yes" : "no" );
        if(( lineLen = getLine( &playerEnteredLine, &linecap, stdin )) == -1 )  {
            exit( EXIT_FAILURE );    /* unrecoverable condition - but finish up nicely as well as quickly */
        }
        else  {
            startIndexOfFirstNonSpaceChar = strspn( playerEnteredLine, " \t" );  /* get index of first non-space character */
            enteredLineChrPtr = playerEnteredLine + startIndexOfFirstNonSpaceChar;
            if( *enteredLineChrPtr == '\0' )  lineLen = 0;     /* playerEnteredLine contains no response - shouldn't happen */
            else if( *enteredLineChrPtr == '\n' )  result = defaultResult;     /* player only typed return, no letters */
            else if( tolower( *enteredLineChrPtr ) == 'y' )  result = TRUE;    /* starts with a 'y' so assume it is yes */
            else if( tolower( *enteredLineChrPtr ) == 'n' )  result = FALSE;   /* starts with a 'n' so assume it is no */
            else  result = defaultResult;
        }
    } while( lineLen <= 0 );
    return( result );
}


long  limitLongValueToEqualOrWithinRange( long  value, long  loBoundary, long  hiBoundary )  {
	return(( value < loBoundary ) ? loBoundary : (( value > hiBoundary ) ? hiBoundary : value ));
}


long  convertOptionStringToLong( long  defltValue, char *  strng )  {
	long  result;
	char *  endPtrStore;

	/* Convert string specified to signed long, if possible */
    result = strtol( strng, &endPtrStore, 10 );		/* base 10 conversion of strng to long integer */
	/* Check on strtol output - did any characters get converted? */
    if( endPtrStore == strng )  result = defltValue;
    return( result );
}


int  getNumberResponseFromPlayer( char *  questionString, int  defaultResult, int loLimit, int  hiLimit )  {
    int  result = defaultResult;    /* default to No response */
    long  numberResponse;
    char *  enteredLineChrPtr;
    ssize_t lineLen;
    size_t  startIndexOfFirstNonSpaceChar;

    do  {
        printf( "%s within range from %d to %d (defaults to %d)? : ", questionString, loLimit, hiLimit, defaultResult );
        if(( lineLen = getLine( &playerEnteredLine, &linecap, stdin )) == -1 )
            exit( EXIT_FAILURE );    /* unrecoverable condition - but finish up nicely as well as quickly */
        else  {
            startIndexOfFirstNonSpaceChar = strspn( playerEnteredLine, " \t" );
            enteredLineChrPtr = playerEnteredLine + startIndexOfFirstNonSpaceChar;
            if( *enteredLineChrPtr == '\0' )  lineLen = 0; /* playerEnteredLine has no response */
            else if( *enteredLineChrPtr == '\n' )  result = defaultResult;
            else {
                numberResponse = convertOptionStringToLong( (long) defaultResult, enteredLineChrPtr );
                numberResponse = limitLongValueToEqualOrWithinRange( numberResponse, loLimit, hiLimit );
                result = ( int ) numberResponse;
            }
        }
    } while( lineLen <= 0 );
    return( result );
}


void  printCurrentGameSettings( void )  {
    printf( "\nThe game settings are; -\n" );
    printf( "%d code letters in use & they are: %s\n", numberOfCodeSymbols, validCodeLetters );
    printf( "The secret code is %d letters long\n", codeWidth );
    printf( "%d attempts to guess the secret code are avaiable\n", totalNumberOfGuesses );
    printf( "Code letters ar%s repeated in the code\n", codeLettersCanBeRepeated ? "e" : "e not" );
}


void  setupValidCodeLetters( int  nmbrOfCodes )  {
    int  cnt;

    for( cnt = 0; cnt < nmbrOfCodes; cnt++ )
        validCodeLetters[ cnt ] = 'A' + cnt;
    validCodeLetters[ nmbrOfCodes ] = '\0';   /* terminate it as a string */
}


void  changeGameSettings( void )  {
    numberOfCodeSymbols = getNumberResponseFromPlayer( "How many letters to choose from",
        DEFAULT_NUMBER_OF_CODE_SYMBOLS, MINIMUM_NUMBER_OF_CODE_SYMBOLS, MAXIMUM_NUMBER_OF_CODE_SYMBOLS );
    printf( "%d code letters in use & they are: ", numberOfCodeSymbols );
    setupValidCodeLetters( numberOfCodeSymbols );
    printf( "%s\n", validCodeLetters );
    codeWidth = getNumberResponseFromPlayer( "How many letters to guess",
        MINIMUM_CODE_LENGTH, MINIMUM_CODE_LENGTH, MAXIMUM_CODE_LENGTH );
    printf( "The code will be %d letters long\n", codeWidth );
    totalNumberOfGuesses = getNumberResponseFromPlayer( "How many guesses",
        MINIMUM_NUMBER_OF_GUESSES, MINIMUM_NUMBER_OF_GUESSES, MAXIMUM_NUMBER_OF_GUESSES );
    printf( "%d attempts to guess the code are avaiable\n", totalNumberOfGuesses );
    if( codeWidth <= numberOfCodeSymbols )
        codeLettersCanBeRepeated = getYesOrNoResponseFromPlayer( "Are repeated code letters allowed in the code?", TRUE );
}


void  generateSecretCodeAllowingRepeatedSymbols( char  secretCode[] )  {
    int  index;
    long  prDivisor;

    prDivisor = ((long) RAND_MAX) / ( long ) numberOfCodeSymbols;    /* Calc Pseudo Random divisor for later use */
    for( index = 0; index < codeWidth; index++ )
        secretCode[ index ] = 'A' + ( int )( rand() / prDivisor );
}


void  generateSecretCodeSansRepeatedSymbols( char *  secretCodePtr )  {
    int  index;
    long  prDivisor;
    int  lettersLeft;
    char *  avlPtr;
    char  availableCodes[ MAXIMUM_NUMBER_OF_CODE_SYMBOLS + 1 ];

    /* Make a copy of the available letters */
    strncpy( availableCodes, validCodeLetters, sizeof( availableCodes ));
    /* Work through the available letters randomly selecting, copying and then removing each choice */
    for( index = 0, lettersLeft = numberOfCodeSymbols; index < codeWidth; index++ )  {
        if( lettersLeft > 1 )  {
            prDivisor = ((long) RAND_MAX) / ( long ) lettersLeft--;
            avlPtr = availableCodes + ( int )( rand() / prDivisor );      /* point at the selected letter */
        }
        else  avlPtr = availableCodes;      /* Only 1 letter left */
        *secretCodePtr++ = *avlPtr;         /* write available letter into secret code string */
        for( ; *avlPtr != '\0'; avlPtr++ )  *avlPtr = *( avlPtr + 1 );   /* compact available letters */
    }
}


void  generateSecretCode( char *  secretCode )  {
    if( codeLettersCanBeRepeated || ( codeWidth > numberOfCodeSymbols ))
        generateSecretCodeAllowingRepeatedSymbols( secretCode );
    else  generateSecretCodeSansRepeatedSymbols( secretCode );
    secretCode[ codeWidth ] = '\0';     /* terminate code so it can be used as a string */
}


void  getGuessForRound( int  roundNumber, char *  playerCodeGuess )  {
    char *  enteredLineChrPtr;
    char *  playerCodeGssChrPtr;

    do  {
        playerCodeGssChrPtr = playerCodeGuess;
        *playerCodeGssChrPtr = '\0';  /* ensure that if getLine() fails playerCodeGuess is a terminated string */
        printf( "Round number %d: Please enter %d letters from the set \"%s\": ",
            roundNumber, codeWidth, validCodeLetters );
        if( getLine( &playerEnteredLine, &linecap, stdin ) == -1 )
            exit( EXIT_FAILURE );    /* unrecoverable condition - but finish up nicely as well as quickly */
        else  {
            enteredLineChrPtr = playerEnteredLine + strspn( playerEnteredLine, " \t" );  /* point at first non-white-space */
            for( ; *enteredLineChrPtr != '\0'; )  {   /* Loop to convert to upper case and copy if in the legal set of char */
                if( strchr( validCodeLetters, toupper( *enteredLineChrPtr )) != NULL )
                    *playerCodeGssChrPtr++ = toupper( *enteredLineChrPtr++ );
                else  enteredLineChrPtr++;     /* step over and ignore anything not in the set of legal code letters */
            }
            *playerCodeGssChrPtr = '\0';  /* ensure playerCodeGuess is a terminated string so strlen() works */
        }
    } while( strlen( playerCodeGuess ) < codeWidth );     /* loop if there is not input from the player */
    playerCodeGuess[ codeWidth ] = '\0';     /* Ensure code guess is expected width in case player has entered too many letters */
}


int  scoreAnyCorrectLetters( char *  score, char *  playrCode, char *  secrtCode )  {
    int  index, scoreIndex;

    for( index = 0, scoreIndex = 0; index < codeWidth; index++ )  {
        if( playrCode[ index ] == secrtCode[ index ] )  {
            score[ scoreIndex++ ] = CORRECT_MARKER;
            playrCode[ index ] = ALREADY_SCORED;   /* make sure this user guess symbol is now ignored */
            secrtCode[ index ] = ALREADY_SCORED;   /* make sure this matched secret symbol is now ignored */
        }
    }
    return( scoreIndex );
}


int  scoreMisplacedLetters( int scoreIndex, char *  score, char *  uCode, char *  sCode )  {
    int  index;
    char *  enteredLineChrPtr;

    for( index = 0; index < codeWidth; index++ )  {
        if(( sCode[ index ] != ALREADY_SCORED ) && ( enteredLineChrPtr = strchr( uCode, sCode[ index ])) != NULL )  {
            score[ scoreIndex++ ] = WRONG_POSITION_MARKER;
            *enteredLineChrPtr = ALREADY_SCORED;   /* make sure this guess cannot be scored again */
        }
    }
    return( scoreIndex );
}


void  determineScore( int  round, char *  secretCode, char *  userCodeGuess, char *  score )  {
    int  index, correctGuesses;
    char  tempUserCodeGuess[ MAXIMUM_CODE_LENGTH + 1 ];     /* Array of code letters in current guess + '\0' */
    char  tempSecretCode[ MAXIMUM_CODE_LENGTH + 1 ];        /* Array of code letters in current guess + '\0' */

    /* Set up score string as no matches */
    for( index = 0; index < codeWidth; index++ )  score[ index ] = INCORRECT_MARKER;
    score[ codeWidth ] = '\0';
    /* Copy userCodeGuess and secretCode so that destructive test doesn't matter */
    strncpy( tempUserCodeGuess, userCodeGuess, sizeof( tempUserCodeGuess ));
    strncpy( tempSecretCode, secretCode, sizeof( tempSecretCode ));
    if(( correctGuesses = scoreAnyCorrectLetters( score, tempUserCodeGuess, tempSecretCode )) < codeWidth )
        if(( index = scoreMisplacedLetters( correctGuesses, score, tempUserCodeGuess, tempSecretCode )) > codeWidth )
            printf( "Error: scoring produced a result that is too long (%d characters)\n", index );
}


int  playGame( void )  {
    int  cnt;
    int  result = FALSE;
    int  roundCnt;
    char  secretCode[ MAXIMUM_CODE_LENGTH + 1 ];        /* Array of code letters player has to discover + '\0' */
    char  userCodeGuess[ MAXIMUM_NUMBER_OF_GUESSES ][ MAXIMUM_CODE_LENGTH + 1 ];    /* Array of code letters in current guess + '\0' */
    char  score[ MAXIMUM_NUMBER_OF_GUESSES ][ MAXIMUM_CODE_LENGTH + 1 ];        /* Array of correct, partially correct and incorrect letters in guess + '\0' */

    generateSecretCode( secretCode );
    printf( "\n" );
    for( roundCnt = 1; roundCnt <= totalNumberOfGuesses; roundCnt++ )  {
        getGuessForRound( roundCnt, userCodeGuess[ roundCnt ] );
        /* Check for a win */
        if(( result = ( strcmp( secretCode, userCodeGuess[ roundCnt ] ) == 0 )))  {    /* A win if strings are equal */
            printf( "Congratulations, you succeeded in specifying \"%s\", which was the secret code!\n", secretCode );
            break;
        }
        else  {
            determineScore( roundCnt, secretCode, userCodeGuess[ roundCnt ], score[ roundCnt ] );
            for( cnt = 1; cnt <= roundCnt; cnt++ )
                printf( "%2d.  %s%s%s\n", cnt, userCodeGuess[ cnt ], resultSeparatorStrng, score[ cnt ] );
            if( roundCnt >= totalNumberOfGuesses )  {
                printf( "Commiserations, you were not successful this time.\n");
                printf( "The secret code was \"%s\"\n", secretCode );
            }
        }
    }
    return( result );
}


void  finishByPrintingScore( void )  {
    /* Finish up */
    if( feof( stdin ))  printf( "\n" );     /* Add blank line if player pressed ^D (EOF)*/
    printf( "\nThank you for playing Mastermind\n" );
    printf( "You played %d game%sand were successful on %d occasion%s\n\n",
        totalGamesPlayed, ( totalGamesPlayed == 1 ) ? " " : "s ",
        playerSuccesses, ( playerSuccesses == 1 ) ? "." : "s." );
    /* Free the storage used by getLine() */
    if( playerEnteredLine != NULL )  {
        free( playerEnteredLine );
        linecap = 0;
    }
}


void  finishUpAfterPlayerInterrupt( int  signalNumber )  {
    printf( "\n" );         /* Add blank line if player pressed ^C, ^\ etc */
    exit( EXIT_FAILURE );
}


void  initializeGlobals( void )  {
    unsigned  seed;
    struct timespec  now;

    setupValidCodeLetters( numberOfCodeSymbols );
    if( clock_gettime( CLOCK_REALTIME, &now ) < 0 )
        seed = ( unsigned ) 0x55AA00FF;     /* perhaps better than 0 if get time fails ? */
    else  seed = ( unsigned )( now.tv_sec ^ now.tv_nsec );  /* Exclusive or */
    srand( seed );  /* initialize the pseudo-random number generator */
}


int  main( int  argc, char *  argv[] )  {
    initializeGlobals();
    atexit( finishByPrintingScore );    /* print out of score at exit time */
	signal( SIGINT, finishUpAfterPlayerInterrupt );     /* Trap Interrupt (Control C) player interrupt */
#ifdef  SIGQUIT
	signal( SIGQUIT, finishUpAfterPlayerInterrupt );    /* Trap Quit (Control \) player interrupt to print like linux */
#endif
#ifdef SIGTERM
	signal( SIGTERM, finishUpAfterPlayerInterrupt );	/* Trap Terminate (Del) player interrupt */
#endif
    printf( "\nHello, Welcome to the game of Mastermind\nThis version of the game uses letter codes instead of colours.\n" );
    do  {
        printCurrentGameSettings();
        if( getYesOrNoResponseFromPlayer( "\nDo you wish to change the Game settings?", FALSE ))
            changeGameSettings();
        playerSuccesses += playGame();
        totalGamesPlayed += 1;
    }  while(( ! feof( stdin )) && getYesOrNoResponseFromPlayer( "\nDo you wish to play again?", TRUE ));
    /* finish up with printed score (N.B. atexit() put finishByPrintingScore() in the exit path */
    return( 0 );
}
