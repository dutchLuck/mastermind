/*
 * M A S T E R M I N D . C
 *
 * C language code to play text mode mastermind
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

#include <stdio.h>  /* printf() getline() */
#include <string.h> /* strspn() strlen() strchr() strcmp() strncpy() */
#include <ctype.h>  /* tolower() */
#include <stdlib.h> /* random() srandom() strtol() */
#include <time.h>   /* clock_gettime() */

#define FALSE ((int) 0)
#define TRUE (! FALSE)
#define MINIMUM_NUMBER_OF_CODES 2
#define DEFAULT_NUMBER_OF_CODES 7
#define MAXIMUM_NUMBER_OF_CODES 20
#define MINIMUM_CODE_LENGTH 4
#define MAXIMUM_CODE_LENGTH 10
#define MINIMUM_NUMBER_OF_GUESSES 7
#define MAXIMUM_NUMBER_OF_GUESSES 20
#define LARGEST_PRNUMBER  ((long) 2147483647)    /* (2^31)-1 is max output by random() */

int  numberOfCodes = DEFAULT_NUMBER_OF_CODES;
int  codeWidth = MINIMUM_CODE_LENGTH;
int  totalNumberOfGuesses = MINIMUM_NUMBER_OF_GUESSES;
int  codeLettersCanBeRepeated = TRUE;
char  secretCode[ MAXIMUM_CODE_LENGTH + 1 ];    /* Array of code letters user has to discover + '\0' */
char  userCodeGuess[ MAXIMUM_CODE_LENGTH + 1 ]; /* Array of code letters in current guess + '\0' */
char  score[ MAXIMUM_CODE_LENGTH + 1 ];         /* Array of correct, partially correct and incorrect letters in guess + '\0' */
char  legalCodeLetters[ MAXIMUM_NUMBER_OF_CODES + 1 ]; /* Array of allowed code letters in current guess + '\0' */
char *  line = NULL;      /* For use by getline() */
size_t linecap = 0;


int  getYesOrNoResponseFromUser( char *  questionString, int  defaultResult )  {
    int  result = defaultResult;    /* default to No response */
    char *  chrPtr;
    ssize_t linelen;
    size_t  startIndexOfFirstNonSpaceChar;

    do  {
        printf( "%s: yes or no (defaults to %s) : ", questionString, defaultResult ? "yes" : "no" );
        if(( linelen = getline(&line, &linecap, stdin )) > 0)  {
            startIndexOfFirstNonSpaceChar = strspn( line, " \t" );
            chrPtr = line + startIndexOfFirstNonSpaceChar;
            if( *chrPtr == '\0' )  linelen = 0; /* line has no response */
            else if( *chrPtr == '\n' )  result = defaultResult;
            else if( tolower( *chrPtr ) == 'y' )  result = TRUE;
            else if( tolower( *chrPtr ) == 'n' )  result = FALSE;
            else  result = defaultResult;
        }
    } while( linelen <= 0 );
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
    if( endPtrStore == strng )
    	result = defltValue;
	return( result );
}


int  getNumberResponseFromUser( char *  questionString, int  defaultResult, int loLimit, int  hiLimit )  {
    int  result = defaultResult;    /* default to No response */
    long  numberResponse;
    char *  chrPtr;
    ssize_t linelen;
    size_t  startIndexOfFirstNonSpaceChar;

    do  {
        printf( "%s within range from %d to %d (defaults to %d)? : ", questionString, loLimit, hiLimit, defaultResult );
        if(( linelen = getline(&line, &linecap, stdin )) > 0)  {
            startIndexOfFirstNonSpaceChar = strspn( line, " \t" );
            chrPtr = line + startIndexOfFirstNonSpaceChar;
            if( *chrPtr == '\0' )  linelen = 0; /* line has no response */
            else if( *chrPtr == '\n' )  result = defaultResult;
            else {
                numberResponse = convertOptionStringToLong( (long) defaultResult, chrPtr );
                numberResponse = limitLongValueToEqualOrWithinRange( numberResponse, loLimit, hiLimit );
                result = ( int ) numberResponse;
            }
        }
    } while( linelen <= 0 );
    return( result );
}


void  printCurrentGameSettings( void )  {
    printf( "\nThe game settings are; -\n" );
    printf( "%d code letters in use & they are: %s\n", numberOfCodes, legalCodeLetters );
    printf( "The secret code is %d letters long\n", codeWidth );
    printf( "%d attempts to guess the secret code are avaiable\n", totalNumberOfGuesses );
    printf( "Code letters ar%s repeated in the code\n", codeLettersCanBeRepeated ? "e" : "e not" );
}


void  changeGameSettings( void )  {
    int  cnt;

    numberOfCodes = getNumberResponseFromUser( "How many letters to choose from",
        DEFAULT_NUMBER_OF_CODES, MINIMUM_NUMBER_OF_CODES, MAXIMUM_NUMBER_OF_CODES );
    printf( "%d code letters in use & they are: ", numberOfCodes );
    for( cnt = 0; cnt < numberOfCodes; cnt++ )
        legalCodeLetters[ cnt ] = 'A' + cnt;
    legalCodeLetters[ numberOfCodes ] = '\0';   /* terminate it as a string */
    printf( "%s\n", legalCodeLetters );
    codeWidth = getNumberResponseFromUser( "How many letters to guess",
        MINIMUM_CODE_LENGTH, MINIMUM_CODE_LENGTH, MAXIMUM_CODE_LENGTH );
    printf( "The code will be %d letters long\n", codeWidth );
    totalNumberOfGuesses = getNumberResponseFromUser( "How many guesses",
        MINIMUM_NUMBER_OF_GUESSES, MINIMUM_NUMBER_OF_GUESSES, MAXIMUM_NUMBER_OF_GUESSES );
    printf( "%d attempts to guess the code are avaiable\n", totalNumberOfGuesses );
    if( codeWidth <= numberOfCodes )
        codeLettersCanBeRepeated = getYesOrNoResponseFromUser( "Are repeated code letters allowed in the code?", TRUE );
}


void  generateSecretCodeAllowingRepeatedSymbols( void )  {
    int  index;
    long  prDivisor;

    prDivisor = ( LARGEST_PRNUMBER / ( long ) numberOfCodes );
    for( index = 0; index < codeWidth; index++ )
        secretCode[ index ] = 'A' + ( int )( random() / ( long ) prDivisor );
}


void  generateSecretCodeSansRepeatedSymbols( char *  secretCodePtr )  {
    int  index;
    long  prDivisor;
    int  lettersLeft;
    char *  avlPtr;
    char  availableCodes[ MAXIMUM_NUMBER_OF_CODES + 1 ];

    /* Make a copy of the available letters */
    strncpy( availableCodes, legalCodeLetters, sizeof( availableCodes ));
    /* Work through the available letters randomly selecting, copying and then removing each choice */
    for( index = 0, lettersLeft = numberOfCodes; index < codeWidth; index++ )  {
        if( lettersLeft > 1 )  {
            prDivisor = ( LARGEST_PRNUMBER / ( long ) lettersLeft-- );
            avlPtr = availableCodes + ( int )( random() / prDivisor );      /* point at the selected letter */
        }
        else  avlPtr = availableCodes;      /* Only 1 letter left */
        *secretCodePtr++ = *avlPtr;         /* write available letter into secretCode string */
        for( ; *avlPtr != '\0'; avlPtr++ )  *avlPtr = *( avlPtr + 1 );   /* compact available letters */
    }
}


void  generateSecretCode( void )  {
    unsigned  seed;
    struct timespec  now;

    if( clock_gettime( CLOCK_REALTIME, &now ) < 0 )
        seed = ( unsigned ) 0x55AA00FF;     /* perhaps better than 0 if get time fails ? */
    else  seed = ( unsigned )( now.tv_sec ^ now.tv_nsec );  /* Exclusive or */
    srandom( seed );
    if( codeLettersCanBeRepeated || ( codeWidth > numberOfCodes ))
        generateSecretCodeAllowingRepeatedSymbols();
    else  generateSecretCodeSansRepeatedSymbols( secretCode );
    secretCode[ codeWidth ] = '\0';     /* terminate code so it can be used as a string */
}


void  getGuessForRound( int  roundNumber )  {
    char *  chrPtr;
    char *  chrBfrPtr;

    do  {
        chrBfrPtr = userCodeGuess;
        *chrBfrPtr = '\0';  /* ensure that if getline() fails userCodeGuess is a terminated string */
        printf( "Round number %d: Please enter %d letters from the set \"%s\": ",
            roundNumber, codeWidth, legalCodeLetters );
        if( getline(&line, &linecap, stdin ) > 0 )  {
            chrPtr = line + strspn( line, " \t" );  /* point at first non-white-space */
            for( ; *chrPtr != '\0'; )  {   /* Loop to convert to upper case and copy if in the legal set of char */
                if( strchr( legalCodeLetters, toupper( *chrPtr )) != NULL )  *chrBfrPtr++ = toupper( *chrPtr++ );
                else  chrPtr++;     /* ignore anything not in the set of legal code letters */
            }
            *chrBfrPtr = '\0';  /* ensure userCodeGuess is a terminated string so strlen() works */
        }
    } while( strlen( userCodeGuess ) < codeWidth );
    userCodeGuess[ codeWidth ] = '\0';     /* truncate user code at proper length in case it was longer than required */
}


int  checkRoundForWin( void )  {
    return( strcmp( secretCode, userCodeGuess ) == 0 );
}


int  scoreAnyCorrectLetters( char *  uCode, char *  sCode )  {
    int  index, cnt;

    for( index = 0, cnt = 0; index < codeWidth; index++ )  {
        if( uCode[ index ] == sCode[ index ] )  {
            score[ cnt++ ] = 'X';
            uCode[ index ] = '-';   /* make sure this user guess symbol is now ignored */
            sCode[ index ] = '-';   /* make sure this matched secret symbol is now ignored */
        }
    }
    return( cnt );
}


int  scoreMisplacedLetters( int scoreIndex, char *  uCode, char *  sCode )  {
    int  index, cnt;
    char *  chrPtr;

    for( index = 0, cnt = scoreIndex; index < codeWidth; index++ )  {
        if(( sCode[ index ] != '-' ) && ( chrPtr = strchr( uCode, sCode[ index ])) != NULL )  {
            score[ cnt++ ] = '0';
            *chrPtr = '-';   /* make sure this guess cannot be scored again */
        }
    }
    return( cnt );
}


void  determineScore( void )  {
    int  index, correctGuesses;
    char  tempUserCodeGuess[ MAXIMUM_CODE_LENGTH + 1 ];     /* Array of code letters in current guess + '\0' */
    char  tempSecretCode[ MAXIMUM_CODE_LENGTH + 1 ];        /* Array of code letters in current guess + '\0' */

    /* Set up score string as no matches */
    for( index = 0; index < codeWidth; index++ )  score[ index ] = '-';
    score[ codeWidth ] = '\0';
    /* Copy userCodeGuess and secretCode so that destructive test doesn't matter */
    strncpy( tempUserCodeGuess, userCodeGuess, sizeof( tempUserCodeGuess ));
    strncpy( tempSecretCode, secretCode, sizeof( tempSecretCode ));
    if(( correctGuesses = scoreAnyCorrectLetters( tempUserCodeGuess, tempSecretCode )) < codeWidth )
        scoreMisplacedLetters( correctGuesses, tempUserCodeGuess, tempSecretCode );
}


int  playGame( void )  {
    int  result;
    int  roundCnt;

    generateSecretCode();
    printf( "\n" );
    for( roundCnt = 1; roundCnt <= totalNumberOfGuesses; roundCnt++ )  {
        getGuessForRound( roundCnt );
        if(( result = checkRoundForWin()))  {
            printf( "Congratulations, you succeeded in specifying \"%s\", which was the correct code!\n", secretCode );
            break;
        }
        else  {
            determineScore();
            printf( "%d.   %s   |   %s\n", roundCnt, userCodeGuess, score );
            if( roundCnt >= totalNumberOfGuesses )  {
                printf( "Commiserations, you were not successful this time.\n");
                printf( "The secret code was \"%s\"\n", secretCode );
            }
        }
    }
    return( result );
}


void initializeGlobals( void )  {
    int  index;

    for( index = 0; index < numberOfCodes; index++ )
        legalCodeLetters[ index ] = 'A' + index;
    legalCodeLetters[ numberOfCodes ] = '\0';   /* terminate it as a string */
}


int  main( int  argc, char *  argv[] )  {
    int  userSuccesses = 0;
    int  totalGamesPlayed = 0;

    initializeGlobals();
    printf( "\nHello, Welcome to the game of Mastermind\nThis version of the game uses letter codes instead of colours.\n" );
    do  {
        printCurrentGameSettings();
        if( getYesOrNoResponseFromUser( "\nDo you wish to change the Game settings?", FALSE ))
            changeGameSettings();
        userSuccesses += playGame();
        totalGamesPlayed += 1;
    }  while( getYesOrNoResponseFromUser( "\nDo you wish to play again?", TRUE ));
    /* Finish up */
    printf( "\nThank you for playing Mastermind\n" );
    printf( "You played %d game%sand were succesful on %d occasion%s\n",
        totalGamesPlayed, ( totalGamesPlayed == 1 ) ? " " : "s ",
        userSuccesses, ( userSuccesses == 1 ) ? "." : "s." );
    return( 0 );
}