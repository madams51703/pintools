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
/* ===================================================================== */
/* Global Variables */
/* ===================================================================== */

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
    string name = RTN_FindNameByAddress(target);
    if (name == "")
        return &invalid;
    else
    {
        return new string(name);
    }
}

VOID Image(IMG img, VOID *v)
{
int loop_count;

   for (loop_count = 0 ; loop_count <= symbol_after_list_count ;  loop_count ++ )
   {
    RTN my_rtn = RTN_FindByName(img, (symbol_after_list[loop_count]).c_str() );
    	if (RTN_Valid(my_rtn))
    	{
       	 RTN_Open(my_rtn);
       	 RTN_InsertCall(my_rtn, IPOINT_AFTER, (AFUNPTR)DoAfter, IARG_FUNCRET_EXITPOINT_VALUE, IARG_PTR, &(symbol_after_list[loop_count] ) , IARG_END);
       	 RTN_Close(my_rtn);
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
    int loop_count;
    int my_length;

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
    int my_length;
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
/* ===================================================================== */

VOID Trace(TRACE trace, VOID *v)
{
    const BOOL print_args = KnobPrintArgs.Value();
    int loop_count; 
    int my_length;
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
                    		INS_InsertPredicatedCall(tail, IPOINT_BEFORE, AFUNPTR(do_call),
                                             IARG_PTR, Target2String(target), IARG_END);
		    }
		    else
		    {
		    	if ( is_symbol_included(code_name) == 1 && exclude_call == 0 )
			{

                    		INS_InsertPredicatedCall(tail, IPOINT_BEFORE, AFUNPTR(do_call),
                                             IARG_PTR, Target2String(target), IARG_END);
		    		
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
				exclude_call = is_symbol_excluded(&call_name);

		    		string is_all = ( string )(symbol_include_list[0] );
		    		if ( is_all.compare("*") == 0 && exclude_call == 0 )
		    		{

                    					INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_indirect),
                       		       			     IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
				}
				else
				{
					if (is_symbol_included(&call_name ) )
					{
                    						INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_indirect),
                       			       			     IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);

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
				exclude_call=is_symbol_excluded(&call_name);
		    		string is_all = ( string )(symbol_include_list[0] );
		    		if ( is_all.compare("*") == 0  && exclude_call == 0 )
		    		{
                    					INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_indirect),
                                   				IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);

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

                    					INS_InsertCall(tail, IPOINT_BEFORE, AFUNPTR(do_call_indirect),
                                   				IARG_BRANCH_TARGET_ADDR, IARG_BRANCH_TAKEN, IARG_END);
					
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

    TraceFile.open(KnobOutputFile.Value().c_str());

    TraceFile << hex;
    TraceFile.setf(ios::showbase);
    
    string trace_header = string("#\n"
                                 "# Call Trace Generated By Pin\n"
                                 "#\n");
    

    TraceFile.write(trace_header.c_str(),trace_header.size());
    
    TRACE_AddInstrumentFunction(Trace, 0);
    IMG_AddInstrumentFunction(Image, 0);
    PIN_AddFiniFunction(Fini, 0);

    // Never returns

    PIN_StartProgram();
    
    return 0;
}

/* ===================================================================== */
/* eof */
/* ===================================================================== */
