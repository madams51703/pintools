/*
 * Copyright 2002-2019 Intel Corporation.
 * 
 * This software is provided to you as Sample Source Code as defined in the accompanying
 * End User License Agreement for the Intel(R) Software Development Products ("Agreement")
 * section 1.L.
 * 
 * This software and the related documents are provided as is, with no express or implied
 * warranties, other than those that are expressly stated in the License.
 */

/*! @file
 *  This file contains an ISA-portable PIN tool for tracing instructions
 */

#include "pin.H"
#include <iostream>
#include <fstream>
#include <string>
#include <locale.h>
#include <wchar.h>
using namespace std;
using std::hex;
using std::cerr;
using std::string;
using std::ios;
using std::endl;
using std::basic_string;
string current_target;
string    symbol_exclude_list [1000000];
int symbol_exclude_list_count=-1;
string    symbol_include_list [1000000];
int symbol_include_list_count=-1;
string    symbol_after_list [1000000];
int symbol_after_list_count=-1;
string    symbol_between_list [1000000];
string    symbol_between_format [1000000];
int       symbol_between_format_num_of_parameter [1000000];
int symbol_between_list_count=-1;
string    symbol_arg_info_list [1000000];
char *    symbol_arg_info_c_str_list [1000000];
char *    symbol_arg_info_format [1000000];
int symbol_arg_info_list_count=-1;
string    symbol_from_lib_list [1000000];
int symbol_from_lib_list_count=-1;

char empty_string[]="";
/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */
    ADDRINT p[15];

std::ofstream TraceFile;

/* ===================================================================== */
/* Commandline Switches */
/* ===================================================================== */

KNOB<string> KnobOutputFile(KNOB_MODE_WRITEONCE, "pintool", "o", "calltrace.out", "specify trace file name");
KNOB<BOOL>   KnobPrintArgs(KNOB_MODE_WRITEONCE, "pintool", "a", "0", "print call arguments ");
//KNOB<BOOL>   KnobPrintArgs(KNOB_MODE_WRITEONCE, "pintool", "i", "0", "mark indirect calls ");

/* ===================================================================== */
/* Print Help Message                                                    */
/* ===================================================================== */

INT32 Usage()
{
    cerr << "This tool produces a call trace." << endl << endl;
    cerr << KNOB_BASE::StringKnobSummary() << endl;
    return -1;
}

string invalid = "invalid_rtn";

VOID DoAfter(ADDRINT ret,string * s)
{
    TraceFile << *s <<"  returns " << ret << endl;
}

VOID report(char * name,char * format,...)
{
	int entries;
	
	va_list argp;
	va_start(argp, format);
	TraceFile << name << "("    ;
	entries=0;
	while (*format != '\0') 
	{
		if (*format == '%') 
		{
      			format++;
      			if (*format == '%') 
      			{	
        			putchar('%');
      			} 
			else if (*format == 'c') 
      			{
				if (entries > 0 )
				{
					TraceFile <<",";
				}
        			TraceFile << (char) va_arg(argp, int);
				entries++;

      		        } 
			else if (*format == 'd') 
      			{
				if (entries > 0 )
				{
					TraceFile <<",";
				}
        			TraceFile << (int) va_arg(argp, int);
				entries++;
      			} 
			else if (*format == 'u') 
      			{
				if (entries > 0 )
				{
					TraceFile <<",";
				}
//				wprintf (L"Text: %s \n",(wchar_t * )  va_arg(argp,  wchar_t * ) );
        			TraceFile <<"\"" << (char * )  va_arg(argp, char * ) << "\"" ;
				entries++;
			}
			else if (*format == 's') 
      			{
				if (entries > 0 )
				{
					TraceFile <<",";
				}
        			TraceFile <<"\"" << (char *) va_arg(argp,char *) << "\"" ;
				entries++;
      			} 
			else if (*format == 'n') 
      				{
					if (entries > 0 )
					{
						TraceFile <<",";
					}
        				TraceFile <<"\"" << "unknown" << "\"" ;
					entries++;
      				} 
				else 
				{
        				fputs("Not implemented", stdout);
      				}		
			} 
			else 
     			{
  				putchar(*format);
    			}

			format++;
		}
		TraceFile << ")" << endl;

    		va_end(argp);

}


VOID report_indirect(char * name,char * format)
{
	int entries;
	
	TraceFile << name << "("    ;
	entries=1;
	while (*format != '\0') 
	{
		if (*format == '%') 
		{
      			format++;
      			if (*format == '%') 
      			{	
        			putchar('%');
      			} 
			else if (*format == 'c') 
      			{
				if (entries > 0 )
				{
					TraceFile <<",";
				}
        			TraceFile << (char) p[entries];
				entries++;

      		        } 
			else if (*format == 'd') 
      			{
				if (entries > 0 )
				{
					TraceFile <<",";
				}
        			TraceFile << (int) p[entries];
				entries++;
      			} 
			else if (*format == 's') 
      			{
				if (entries > 0 )
				{
					TraceFile <<",";
				}
        			TraceFile <<"\"" << (char *) p[entries] << "\"" ;
				entries++;
      			} 
			else if (*format == 'n') 
      				{
					if (entries > 0 )
					{
						TraceFile <<",";
					}
        				TraceFile <<"\"" << "unknown" << "\"" ;
					entries++;
      				} 
				else 
				{
        				fputs("Not implemented", stdout);
      				}		
			} 
			else 
     			{
  				putchar(*format);
    			}

			format++;
		}
		TraceFile << ")" << endl;


}
VOID Arg1Before(string * name, ...)
{
	int found_arg;
	int loop_count;
    ADDRINT p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11;
        char * call_name;
 va_list argp;
    va_start(argp, name);
    p1= va_arg(argp, ADDRINT);
    p2= va_arg(argp, ADDRINT);
    p3= va_arg(argp, ADDRINT);
    p4= va_arg(argp, ADDRINT);
    p5= va_arg(argp, ADDRINT);
    p6= va_arg(argp, ADDRINT);
    p7= va_arg(argp, ADDRINT);
    p8= va_arg(argp, ADDRINT);
    p9= va_arg(argp, ADDRINT);
    p10= va_arg(argp, ADDRINT);
    p11= va_arg(argp, ADDRINT);
    va_end(argp);
    call_name = strdup(name->c_str() );
	found_arg=-1;
        for ( loop_count=0 ; loop_count <= symbol_arg_info_list_count  ; loop_count++ )
	{
		if ( *name == symbol_arg_info_list[loop_count]  )
		{
			found_arg=loop_count;

		}
	}

    if (found_arg > -1)
    {
    	report(call_name,symbol_arg_info_format[found_arg],p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11);
    }
    else
    {
    	report(call_name,empty_string,p1,p2,p3,p4,p5,p6,p7,p8,p9,p10,p11);
    }

}

int is_symbol_excluded(const string * string_to_test)
{
int loop_count;
int my_length;

	for ( loop_count=0 ; loop_count <= symbol_exclude_list_count  ; loop_count++ )
	{
		string exclude_string = ( string )(symbol_exclude_list[loop_count]) ;
		my_length = exclude_string.length();
		if ( (*string_to_test).compare(0,my_length,exclude_string,0,my_length) == 0 )
			{
				return 1;	
			}
	}
	
return 0;

}

int is_symbol_after(const string * string_to_test)
{
int loop_count;
int my_length;

	for ( loop_count=0 ; loop_count <= symbol_after_list_count  ; loop_count++ )
	{
		string after_string = ( string )(symbol_after_list[loop_count]) ;
		my_length = after_string.length();
		if ( (*string_to_test).compare(0,my_length,after_string,0,my_length) == 0 )
			{
				return 1;	
			}
	}
return 0;
}

int is_symbol_included(const string * string_to_test)
{
int loop_count;
int my_length;

	for ( loop_count=0 ; loop_count <= symbol_include_list_count  ; loop_count++ )
	{
		string include_string = ( string )(symbol_include_list[loop_count]) ;
		my_length = include_string.length();
		if ( (*string_to_test).compare(0,my_length,include_string,0,my_length) == 0 )
			{
				return 1;	
			}
	}
return 0;

}
/* ===================================================================== */
const string *Target2String(ADDRINT target)
{
int loop_count;
    string name = RTN_FindNameByAddress(target);
	for ( loop_count=0 ; loop_count <= symbol_from_lib_list_count  ; loop_count++ )
	{
		if (name == symbol_from_lib_list[loop_count] )
		{
			TraceFile << "FOUND " << name << " at ADDRINT " << target << endl;
		}

	}
    
    if (name == "")
        return &invalid;
    else
    {
        return new string(name);
    }
}


VOID Image(IMG img, VOID *v)
{
    // Instrument the malloc() and free() functions.  Print the input argument
    // of each malloc() or free(), and the return value of malloc().
    //
    //  Find the malloc() function.
    // Find the free() function.
int loop_count;
//int my_length;

        for ( loop_count=0 ; loop_count <= symbol_from_lib_list_count  ; loop_count++ )
	{
		string include_string = ( string )(symbol_from_lib_list[loop_count]) ;

    		RTN freeRtn = RTN_FindByName(img, include_string.c_str());
    		if (RTN_Valid(freeRtn))
    		{
        		RTN_Open(freeRtn);
			string this_image  = IMG_Name(img);
			ADDRINT this_img_addr =RTN_Address(freeRtn);
			TraceFile << "FOUND " << include_string << " From " << this_image << " at ADDRINT "<< this_img_addr << endl;
        		// Instrument free() to print the input argument value.
//        		RTN_InsertCall(freeRtn, IPOINT_BEFORE, (AFUNPTR)Arg1Before,
//				IARG_ADDRINT,  symbol_arg_info_c_str_list[loop_count] ,
//				IARG_ADDRINT , symbol_arg_info_format[loop_count],
//				IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
//				IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
//				IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
//				IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
//				IARG_FUNCARG_ENTRYPOINT_VALUE, 4,
//				IARG_FUNCARG_ENTRYPOINT_VALUE, 5,
//				IARG_FUNCARG_ENTRYPOINT_VALUE, 6,
//				IARG_FUNCARG_ENTRYPOINT_VALUE, 7,
//				IARG_FUNCARG_ENTRYPOINT_VALUE, 8,
//				IARG_FUNCARG_ENTRYPOINT_VALUE, 9,
//				IARG_FUNCARG_ENTRYPOINT_VALUE, 10,
//				IARG_FUNCARG_ENTRYPOINT_VALUE, 11,
//                      		IARG_END);

        		RTN_Close(freeRtn);
    		}
	}
}
/* ===================================================================== */

VOID  do_call_args(const string *s, ADDRINT arg0)
{
    TraceFile << *s << "(" << arg0 << ",...)" << endl;
}

/* ===================================================================== */

VOID  do_call_args_indirect(ADDRINT target, BOOL taken, ADDRINT arg0)
{
int exclude_call;
    exclude_call=0;
    //int loop_count;
    //int my_length;

    if( !taken ) return;
    
    const string *s = Target2String(target);
    exclude_call = is_symbol_excluded(s);

    if ( exclude_call == 0 )
    {
    	do_call_args(s, arg0);
    }

    if (s != &invalid)
        delete s;
}

/* ===================================================================== */

VOID  do_call(const string *s)
{
    TraceFile << *s << endl;
}

/* ===================================================================== */

VOID  do_call_indirect(ADDRINT target, BOOL taken)
{
int exclude_call;
    exclude_call=0;
    int loop_count;
    //int my_length;
    string is_all = ( string )(symbol_include_list[0] );

    if( !taken ) return;
    const string *s = Target2String(target);
    exclude_call = is_symbol_excluded(s);
    if ( exclude_call == 0 )
    {
	if ( is_all.compare("*")  == 0 && exclude_call == 0 )
	{
    		do_call( s );
	}
	else
	{
		for ( loop_count=0 ; loop_count <= symbol_include_list_count  && exclude_call ==0 ; loop_count++ )
		{
			string my_string = ( string )(symbol_include_list[loop_count]) ;
		    	int my_length = my_string.length();
		   	if ( (*s).compare(0,my_length,my_string,0,my_length) == 0 )
			{
    				do_call( s );
			}
		}
	}
    }
    
    if (s != &invalid)
        delete s;
}

VOID  do_call_indirect_var(char * calling_name,ADDRINT target, BOOL taken,...)
{
int exclude_call;
    exclude_call=0;
    char * name;
    int found_arg;
    int loop_count;
    //int my_length;
    string is_all = ( string )(symbol_include_list[0] );

    if( !taken ) return;

    TraceFile << calling_name << " -> " ;
    const string *s = Target2String(target);
    exclude_call = is_symbol_excluded(s);
    name = strdup(&(*s->c_str() ));
    va_list argp;
    va_start(argp, taken);
    p[1]= va_arg(argp, ADDRINT);
    p[2]= va_arg(argp, ADDRINT);
    p[3]= va_arg(argp, ADDRINT);
    p[4]= va_arg(argp, ADDRINT);
    p[5]= va_arg(argp, ADDRINT);
    p[6]= va_arg(argp, ADDRINT);
    p[7]= va_arg(argp, ADDRINT);
    p[8]= va_arg(argp, ADDRINT);
    p[9]= va_arg(argp, ADDRINT);
    p[10]= va_arg(argp, ADDRINT);
    p[11]= va_arg(argp, ADDRINT);
    va_end(argp);
	found_arg=-1;
        for ( loop_count=0 ; loop_count <= symbol_arg_info_list_count  ; loop_count++ )
	{
		if ( *s == symbol_arg_info_list[loop_count]  )
		{
			found_arg=loop_count;

		}
	}

    if ( exclude_call == 0 )
    {
	if ( is_all.compare("*")  == 0 && exclude_call == 0 )
	{
		if ( found_arg> -1 )
		{
			report_indirect(name,symbol_arg_info_format[found_arg]);
		}
		else
		{
			report_indirect(name,empty_string);
		}
	
	}
	else
	{
		for ( loop_count=0 ; loop_count <= symbol_include_list_count  && exclude_call ==0 ; loop_count++ )
		{
			string my_string = ( string )(symbol_include_list[loop_count]) ;
		    	int my_length = my_string.length();
		   	if ( (*s).compare(0,my_length,my_string,0,my_length) == 0 )
			{
				if (found_arg > -1 )
				{
					report_indirect(name,symbol_arg_info_format[found_arg]);
				}
				else
				{
					report_indirect(name,empty_string);

				}	
			}
		}
	}
    }
    
    if (s != &invalid)
        delete s;
}
/* ===================================================================== */

VOID Trace(TRACE trace, VOID *v)
{
    const BOOL print_args = KnobPrintArgs.Value();
    //int my_length;
    int exclude_call=0; 
    for (BBL bbl = TRACE_BblHead(trace); BBL_Valid(bbl); bbl = BBL_Next(bbl))
    {
        exclude_call=0;
        INS tail = BBL_InsTail(bbl);
        
        if( INS_IsCall(tail) )
        {
            if( INS_IsDirectControlFlow(tail) )
            {
                const ADDRINT target = INS_DirectControlFlowTargetAddress(tail);
                const string  * code_name =  Target2String(target); 

		if( print_args )
                {
                    INS_InsertPredicatedCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_args),
                                             IARG_PTR, Target2String(target), IARG_FUNCARG_CALLSITE_VALUE, 0, IARG_END);
                }
                else
                {
		    exclude_call = is_symbol_excluded(code_name);
		    string is_all = ( string )(symbol_include_list[0] );
		    if ( is_all.compare("*")  == 0 && exclude_call == 0 )
		    {
                    			INS_InsertPredicatedCall(tail, IPOINT_BEFORE, AFUNPTR(Arg1Before),
                                             IARG_PTR,  code_name,
						IARG_FUNCARG_CALLSITE_VALUE, 0,
						IARG_FUNCARG_CALLSITE_VALUE, 1,
						IARG_FUNCARG_CALLSITE_VALUE, 2,
						IARG_FUNCARG_CALLSITE_VALUE, 3,
						IARG_FUNCARG_CALLSITE_VALUE, 4,
						IARG_FUNCARG_CALLSITE_VALUE, 5,
						IARG_FUNCARG_CALLSITE_VALUE, 6,
						IARG_FUNCARG_CALLSITE_VALUE, 7,
						IARG_FUNCARG_CALLSITE_VALUE, 8,
						IARG_FUNCARG_CALLSITE_VALUE, 9,
						IARG_FUNCARG_CALLSITE_VALUE, 10,
						IARG_FUNCARG_CALLSITE_VALUE, 11,
						IARG_END);

		    }
		    else
		    {
		    	if ( is_symbol_included(code_name) == 1 && exclude_call == 0 )
			{

		    		
                    			INS_InsertPredicatedCall(tail, IPOINT_BEFORE, AFUNPTR(Arg1Before),
                                             IARG_PTR,  code_name,
	     				     IARG_FUNCARG_CALLSITE_VALUE, 0,
				IARG_FUNCARG_CALLSITE_VALUE, 1,
				IARG_FUNCARG_CALLSITE_VALUE, 2,
				IARG_FUNCARG_CALLSITE_VALUE, 3,
				IARG_FUNCARG_CALLSITE_VALUE, 4,
				IARG_FUNCARG_CALLSITE_VALUE, 5,
				IARG_FUNCARG_CALLSITE_VALUE, 6,
				IARG_FUNCARG_CALLSITE_VALUE, 7,
				IARG_FUNCARG_CALLSITE_VALUE, 8,
				IARG_FUNCARG_CALLSITE_VALUE, 9,
				IARG_FUNCARG_CALLSITE_VALUE, 10,
				IARG_FUNCARG_CALLSITE_VALUE, 11,
					     IARG_END);

		    	}
		   }
                }
            }
            else
            {
                if( print_args )
                {
                    INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_args_indirect),
                                   IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN,  IARG_FUNCARG_CALLSITE_VALUE, 0, IARG_END);
                }
                else
                {
			RTN my_rtn = INS_Rtn(tail);
			if (RTN_Valid(my_rtn) )
			{


				string call_name = RTN_Name(my_rtn);
				char * calling;
				calling = strdup(call_name.c_str() );
				exclude_call = is_symbol_excluded(&call_name);

		    		string is_all = ( string )(symbol_include_list[0] );
		    		if ( is_all.compare("*") == 0 && exclude_call == 0 )
		    		{

       		             			INS_InsertPredicatedCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_indirect_var),
							IARG_PTR,calling,
       		                                	IARG_BRANCH_TARGET_ADDR,
							IARG_BRANCH_TAKEN,
							IARG_FUNCARG_CALLSITE_VALUE, 0,
							IARG_FUNCARG_CALLSITE_VALUE, 1,
							IARG_FUNCARG_CALLSITE_VALUE, 2,
							IARG_FUNCARG_CALLSITE_VALUE, 3,
							IARG_FUNCARG_CALLSITE_VALUE, 4,
							IARG_FUNCARG_CALLSITE_VALUE, 5,
							IARG_FUNCARG_CALLSITE_VALUE, 6,
							IARG_FUNCARG_CALLSITE_VALUE, 7,
							IARG_FUNCARG_CALLSITE_VALUE, 8,
							IARG_FUNCARG_CALLSITE_VALUE, 9,
							IARG_FUNCARG_CALLSITE_VALUE, 10,
							IARG_FUNCARG_CALLSITE_VALUE, 11,
					     			IARG_END);

				
				
				
				}
					
				else
				{
					if (is_symbol_included(&call_name ) )
					{


                    					INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_indirect_var),
									IARG_PTR,calling,
                                             				IARG_BRANCH_TARGET_ADDR,
									IARG_BRANCH_TAKEN,
								IARG_FUNCARG_CALLSITE_VALUE, 0,
								IARG_FUNCARG_CALLSITE_VALUE, 1,
								IARG_FUNCARG_CALLSITE_VALUE, 2,
								IARG_FUNCARG_CALLSITE_VALUE, 3,
								IARG_FUNCARG_CALLSITE_VALUE, 4,
								IARG_FUNCARG_CALLSITE_VALUE, 5,
								IARG_FUNCARG_CALLSITE_VALUE, 6,
								IARG_FUNCARG_CALLSITE_VALUE, 7,
								IARG_FUNCARG_CALLSITE_VALUE, 8,
								IARG_FUNCARG_CALLSITE_VALUE, 9,
								IARG_FUNCARG_CALLSITE_VALUE, 10,
								IARG_FUNCARG_CALLSITE_VALUE, 11,
					     				IARG_END);


					}
					
				}
               	 	}
                }
                
            }
        }
        else
        {
            // sometimes code is not in an image
            RTN rtn = TRACE_Rtn(trace);
             
            // also track stup jumps into share libraries
            if( RTN_Valid(rtn) && !INS_IsDirectControlFlow(tail) && ".plt" == SEC_Name( RTN_Sec( rtn ) ))
            {
                if( print_args )
                {
                    INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_args_indirect),
                                   IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN,  IARG_FUNCARG_CALLSITE_VALUE, 0, IARG_END);
                }
                else
                {
			if (RTN_Valid(rtn) )
			{
				string call_name=RTN_Name(rtn);
				char * calling;
				calling = strdup(call_name.c_str() );
				exclude_call=is_symbol_excluded(&call_name);
		    		string is_all = ( string )(symbol_include_list[0] );
		    		if ( is_all.compare("*") == 0  && exclude_call == 0 )
		    		{


                    				INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_indirect_var),
								IARG_PTR, calling,
								IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 4,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 5,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 6,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 7,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 8,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 9,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 10,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 11,
					     			IARG_END);


				

					if (is_symbol_after(&call_name) == 1 )
					{
                    				INS_InsertCall(tail, IPOINT_AFTER, AFUNPTR(DoAfter),
                                            	 IARG_FUNCRET_EXITPOINT_VALUE,IARG_PTR, &call_name, IARG_END);

					}
				}
				else
				{
					if (is_symbol_included(&call_name ) )
					{

                    				INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_indirect_var),
							IARG_PTR, calling,
							IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 0,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 1,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 2,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 3,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 4,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 5,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 6,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 7,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 8,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 9,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 10,
							IARG_FUNCARG_ENTRYPOINT_VALUE, 11,
					     			IARG_END);

					
						if (is_symbol_after(&call_name) == 1 )
						{	
                    					INS_InsertCall(tail, IPOINT_AFTER, AFUNPTR(DoAfter),
                                            		 IARG_FUNCRET_EXITPOINT_VALUE,IARG_PTR, &call_name, IARG_END);

						}	
					}
				}
			      }
			   }
			}
		    
                
    		}
	}    

}

// Pin calls this function every time a new img is loaded
// It can instrument the image, but this example does not
// Note that imgs (including shared libraries) are loaded lazily

VOID ImageLoad(IMG img, VOID *v)
{
    TraceFile << "Loading " << IMG_Name(img) << ", Image id = " << IMG_Id(img) << endl;
}

    // Pin calls this function every time a new img is unloaded
    // You can't instrument an image that is about to be unloaded

VOID ImageUnload(IMG img, VOID *v)
{
        TraceFile << "Unloading " << IMG_Name(img) << endl;
}


/* ===================================================================== */

VOID Fini(INT32 code, VOID *v)
{
    TraceFile << "# eof" << endl;
    
    TraceFile.close();
}

/* ===================================================================== */
/* Main                                                                  */
/* ===================================================================== */

int  main(int argc, char *argv[])
{
string line;
    PIN_InitSymbols();

    if( PIN_Init(argc,argv) )
    {
        return Usage();
    }
    
   // Load argument data that we know about 
	std::size_t current,previous = 0;
//	std::size_t npos = 0;
	string format;
	string format_part;
	int entry = 0;
	ifstream arg_infofile ("arg_info.txt");
	if (arg_infofile.is_open())
	{
		while ( getline (arg_infofile,line) )
	  	{
			entry=0;
    			symbol_arg_info_list_count++;

			format = "";
			current=0;
			previous=0;
			current = line.find(',');
			while (current != std::string::npos) {
				if ( entry == 0 )
				{
    					symbol_arg_info_list[symbol_arg_info_list_count] =   line.substr(previous, current - previous);
					symbol_arg_info_c_str_list[symbol_arg_info_list_count] = strdup ( (line.substr(previous, current - previous) ).c_str() );
				}
				else
				{
					format +=  line.substr(previous, current - previous) ;
				}
				entry += 1;
			        previous = current + 1;
				current = line.find(',', previous);
			}
			if ( entry == 0 )
			{
    				symbol_arg_info_list[symbol_arg_info_list_count] =   line.substr(previous, current - previous);
			}
			else
			{
				format +=  line.substr(previous, current - previous) ;
			}
			symbol_arg_info_format [symbol_arg_info_list_count] = strdup(format.c_str() );
			
		}
		arg_infofile.close();
	}
	ifstream myfile ("include.txt");
	if (myfile.is_open())
	{
		while ( getline (myfile,line) )
	  	{
    			symbol_include_list_count++;
    			symbol_include_list[symbol_include_list_count] =  line;
		}
		myfile.close();
	}

	ifstream excludefile ("exclude.txt");
	if (excludefile.is_open())
	{
		while ( getline (excludefile,line) )
	  	{
    			symbol_exclude_list_count++;
    			symbol_exclude_list[symbol_exclude_list_count] =  line;
		}
		excludefile.close();
	}

	ifstream afterfile ("after.txt");
	if (afterfile.is_open())
	{
		while ( getline (afterfile,line) )
	  	{
    			symbol_after_list_count++;
    			symbol_after_list[symbol_after_list_count] =  line;
		}
		afterfile.close();
	}

	ifstream from_lib_file ("from_lib.txt");
	if (from_lib_file.is_open())
	{
		while ( getline (from_lib_file,line) )
	  	{
    			symbol_from_lib_list_count++;
    			symbol_from_lib_list[symbol_from_lib_list_count] =  line;
		}
		from_lib_file.close();
	}

    TraceFile.open(KnobOutputFile.Value().c_str());

    TraceFile << hex;
    TraceFile.setf(ios::showbase);
    
    string trace_header = string("#\n"
                                 "# Call Trace Generated By Pin\n"
                                 "#\n");
    

    TraceFile.write(trace_header.c_str(),trace_header.size());
    
    TRACE_AddInstrumentFunction(Trace, 0);


	// Register ImageLoad to be called when an image is loaded
	IMG_AddInstrumentFunction(ImageLoad, 0);

	// Register ImageUnload to be called when an image is unloaded
	IMG_AddUnloadFunction(ImageUnload, 0);



    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
