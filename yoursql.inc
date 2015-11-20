/*
* ================
* SAMP - YourSQL (build 5)
* The easiest and one of the fastest SQLITE manager, library. Requires you with no SQL knowledge!
* ================
*
* ================
* Create by: Gammix
* ================
*
* ================
* Special thanks to: Slice (for BUD include as refrence), Y_Less (for string argument functions)
* ================
*
* ================
* Functions/Stocks:
	native SQL:yoursql_open(const name[]);
	native bool:yoursql_close(SQL:id);
	native bool:yoursql_verify_table(SQL:id, const table[]);
	native bool:yoursql_verify_column(SQL:id, const dest[], SQLType:type, {_, Float}:...);
	native SQLRow:yoursql_get_row(SQL:id, const dest[], const fmat[], {_, Float}:...);
	native bool:yoursql_set_row(SQL:id, const dest[], const fmat[], {_, Float}:...);
	native bool:yoursql_delete_row(SQL:id, const dest[], SQLRow:rowid);
	native bool:yoursql_set_field(SQL:id, const dest[], SQLRow:rowid, set[]);
	native bool:yoursql_set_field_int(SQL:id, const dest[], SQLRow:rowid, set);
	native bool:yoursql_set_field_float(SQL:id, const dest[], SQLRow:rowid, Float:set);
	native yoursql_get_field(SQL:id, const dest[], SQLRow:rowid, get[], maxlength = sizeof(get));
	native yoursql_get_field_int(SQL:id, const dest[], SQLRow:rowid);
	native Float:yoursql_get_field_float(SQL:id, const dest[], SQLRow:rowid);
	native bool:yoursql_multiset_fields(SQL:id, const dest[], SQLRow:rowid, const fmat[], {_, Float}:...);
	native bool:yoursql_multiget_fields(SQL:id, const dest[], SQLRow:rowid, const fmat[], {_, Float}:...);
	native yoursql_num_fields(SQL:id, const dest[]);
	native yoursql_num_rows(SQL:id, const dest[]);
	native yoursql_sort_int(SQL:id, const dest[], keys[], values[], limit = sizeof(keys), bool:desc = true);
	native yoursql_sort_float(SQL:id, const dest[], keys[], Float:values[], limit = sizeof(keys), bool:desc = true);
	native DBResult:yoursql_query(SQL:id, const query[], {_, Float}:...);
*
* ================
*/

#if defined yoursql_included
	#endinput
#endif
#define yoursql_included

#if ! defined MAX_DATABASE_FILES
	#define MAX_DATABASE_FILES (5)
#endif

#if ! defined MAX_DATABASE_NAME
	#define MAX_DATABASE_NAME (45)
#endif

#if ! defined MAX_DATABASE_ALIVETIME
	#define MAX_DATABASE_ALIVETIME (2000)
#endif

#if ! defined MAX_DATABASE_ENTRY
	#define MAX_DATABASE_ENTRY (1024)
#endif

#if ! defined MAX_DATABASE_QUERY
	#define MAX_DATABASE_QUERY (3024)
#endif

#if ! defined SQL_INVALID_DB
	#define SQL_INVALID_DB (SQL:-1)
#endif

#if ! defined SQL_INVALID_ROW
	#define SQL_INVALID_ROW (SQLRow:0)
#endif

forward __yoursql_skip_no_response();
public 	__yoursql_skip_no_response()
{
	format("", 0, "");
	strcat("", "", 0);
}

enum SQLType
{
	SQL_STRING,
	SQL_NUMBER,
	SQL_FLOAT
};

enum SQLDatabase
{
	DB:	DB_Id,
		DB_Name[MAX_DATABASE_NAME],
		DB_Timer
};

stock static
	yoursql_Database[SQL:MAX_DATABASE_FILES][SQLDatabase],
	yoursql_Query[MAX_DATABASE_QUERY]
;

stock static bool:yoursql_get_db(SQL:id)
{
	if (yoursql_Database[id][DB_Timer] == -1)
	{
	    yoursql_Database[id][DB_Id] = db_open(yoursql_Database[id][DB_Name]);
		if (yoursql_Database[id][DB_Id] == DB:0)
		{
		    return false;
		}
		else
		{
			db_query(yoursql_Database[id][DB_Id], "PRAGMA synchronous = 0");
		}
	}
	else
	{
		KillTimer(yoursql_Database[id][DB_Timer]);
	}

	yoursql_Database[id][DB_Timer] = SetTimerEx("OnDBExpire", MAX_DATABASE_ALIVETIME, false, "i", _:id);

	return true;
}

forward OnDBExpire(SQL:i);
public  OnDBExpire(SQL:i)
{
	yoursql_Database[i][DB_Timer] = -1;

	db_close(yoursql_Database[i][DB_Id]);
	yoursql_Database[i][DB_Id] = DB:0;
}

stock static yoursql_split_in_2(const src[], str1[], str2[], const delmin[] = "=", size1 = sizeof(str1), size2 = sizeof(str2))
{
    new
		pos = strfind(src, delmin)
	;
	strmid(str1, src, 0, (src[pos - 1] == ' ') ? (pos - 1) : (pos), size1);
	strmid(str2, src, (src[pos + 1] == ' ') ? (pos + 2) : (pos + 1), strlen(src), size2);
}

stock static yoursql_getstringarg(dest[], arg, len = sizeof (dest))
{
    // Get the address of the previous function's stack.  First get the index of
    // the argument required.
    #emit LOAD.S.pri arg
    
    // Then convert that number to bytes from cells.
    #emit SMUL.C     4
    // Get the previous function's frame.  Stored in variable 0 (in the current
    // frame).  Parameters are FRM+n+12, locals are FRM-n, previous frame is
    // FRM+0, return address is FRM+4, parameter count is FRM+8.  We could add
    // checks that "arg * 4 < *(*(FRM + 0) + 8)", for the previous frame parameter
    // count (in C pointer speak).
    #emit LOAD.S.alt 0
    
    // Add the frame pointer to the argument offset in bytes.
    #emit ADD
    
    // Add 12 to skip over the function header.
    #emit ADD.C      12
    
    // Load the address stored in the specified address.
    #emit LOAD.I
    
    // Push the length for "strcat".
    #emit PUSH.S     len
    
    // Push the address we just determined was the source.
    #emit PUSH.pri
    
    // Load the address of the destination.
    #emit LOAD.S.alt dest
    
    // Blank the first cell so "strcat" behaves like "strcpy".
    #emit CONST.pri  0
    
    // Store the loaded number 0 to the loaded address.
    #emit STOR.I
    
    // Push the loaded address.
    #emit PUSH.alt
    
    // Push the number of parameters passed (in bytes) to the function.
    #emit PUSH.C     12
    
    // Call the function.
    #emit SYSREQ.C   strcat
    
    // Restore the stack to its level before we called this native.
    #emit STACK      16
}

stock static yoursql_setstringarg(arg, const value[], len = sizeof (value))
{
	new
		address
	;

	// Get the address of the previous function's stack.  First get the index of
    // the argument required.
    #emit LOAD.S.pri arg
    
    // Then convert that number to bytes from cells.
    #emit SMUL.C     4
    
    // Get the previous function's frame.
	#emit LOAD.S.alt 0
	
	// Add the frame pointer to the argument offset in bytes.
    #emit ADD
    
    // Add 12 to skip over the function header.
    #emit ADD.C      12
    
    // Load the address stored in the specified address.
    #emit LOAD.I
	#emit STOR.S.PRI address

	// Push the length (last argument first)
	#emit PUSH.S     len

	// Push the new value (source) value
	#emit PUSH.S     value

	// Blank out the first cell of the argument
	#emit CONST.pri  0
	#emit SREF.S.pri address

	// Push the destination
	#emit PUSH.S     address

	// Push the number of parameters passed (in bytes) to the function.
	#emit PUSH.C     12

	// Call the function.
	#emit SYSREQ.C   strcat

	// Restore the stack to its level before we called this native.
	#emit STACK      16
}

stock static yoursql_returnstringarg(const fmat[], STATIC_ARGS)
{
 	static
 	    out[MAX_DATABASE_QUERY],
		size = MAX_DATABASE_QUERY
	;

	new
		num_args,
		arg_start,
		arg_end
	;

	// Get the pointer to the number of arguments to the last function.
	#emit LOAD.S.pri   0
	#emit ADD.C        8
	#emit MOVE.alt

	// Get the number of arguments.
	#emit LOAD.I
	#emit STOR.S.pri   num_args

	// Get the variable arguments (end).
	#emit ADD
	#emit STOR.S.pri   arg_end

	// Get the variable arguments (start).
	#emit LOAD.S.pri   STATIC_ARGS
	#emit SMUL.C       4
	#emit ADD
	#emit STOR.S.pri   arg_start

	// Using an assembly loop here screwed the code up as the labels added some
	// odd stack/frame manipulation code...
	while (arg_end != arg_start)
	{
		#emit MOVE.pri
		#emit LOAD.I
		#emit PUSH.pri
		#emit CONST.pri    4
		#emit SUB.alt
		#emit STOR.S.pri   arg_end
	}

	// Push the additional parameters.
	#emit PUSH.S       fmat
	#emit PUSH         size
	#emit PUSH.C       out

	// Push the argument count.
	#emit LOAD.S.pri   num_args
	#emit ADD.C        12
	#emit LOAD.S.alt   STATIC_ARGS
	#emit XCHG
	#emit SMUL.C       4
	#emit SUB.alt
	#emit PUSH.pri
	#emit MOVE.alt

	// This gets confused if you have a local variable of the same name as it
	// seems to factor in them first, so you get the offset of the local
	// variable instead of the index of the native.
	#emit SYSREQ.C     format

	// Clear the stack.
	#emit CONST.pri    4
	#emit ADD
	#emit MOVE.alt

	// The three lines above get the total stack data size, now remove it.
	#emit LCTRL        4
	#emit ADD
	#emit SCTRL        4

	// Now do the real return.
	return out;
}

stock SQL:yoursql_open(const name[])
{
	for (new id; id < MAX_DATABASE_FILES; id++)
	{
	    if (! yoursql_Database[SQL:id][DB_Name][0])
	    {
	        format(yoursql_Database[SQL:id][DB_Name], MAX_DATABASE_NAME, name);
	        yoursql_Database[SQL:id][DB_Id] = DB:0;
	        yoursql_Database[SQL:id][DB_Timer] = -1;

	        if (! yoursql_get_db(SQL:id))
	        {
	            yoursql_Database[SQL:id][DB_Name][0] = EOS;
	            return SQL_INVALID_DB;
	        }
	        return SQL:id;
	    }
	}

	return SQL_INVALID_DB;
}

stock bool:yoursql_close(SQL:id)
{
    if (! yoursql_Database[id][DB_Name][0])
    {
        return false;
	}

	if (yoursql_Database[id][DB_Timer] != -1)
	{
	    KillTimer(yoursql_Database[id][DB_Timer]);
	    OnDBExpire(id);
	}

	yoursql_Database[id][DB_Name][0] = EOS;

	id = SQL_INVALID_DB;

	return true;
}

stock bool:yoursql_verify_table(SQL:id, const dest[])
{
    if (! yoursql_get_db(id))
    {
        return false;
    }

    yoursql_Query[0] = EOS;
	strcat(yoursql_Query, "CREATE TABLE `");
	strcat(yoursql_Query, dest);
	strcat(yoursql_Query, "` (`ROW_ID` INTEGER PRIMARY KEY)");
	db_free_result(db_query(yoursql_Database[id][DB_Id], yoursql_Query));

 	return true;
}

stock bool:yoursql_verify_column(SQL:id, const dest[], SQLType:type, {_, Float}:...)
{
    new
        split[2][MAX_DATABASE_NAME * 2]
	;
	yoursql_split_in_2(dest, split[0], split[1], "/", MAX_DATABASE_NAME, MAX_DATABASE_NAME);
	if (! split[0][0] || ! split[1][0])
	{
	    return false;
	}

    if (! yoursql_get_db(id))
    {
        return false;
    }

	switch (type)
	{
	    case SQL_STRING:
	    {
	        new
				default_value[MAX_DATABASE_ENTRY * 2]
			;
			if (numargs() == 4)
			{
				yoursql_getstringarg(default_value, 3);
				default_value[MAX_DATABASE_ENTRY - 1] = EOS;
			}

			format(yoursql_Query, MAX_DATABASE_QUERY, "ALTER TABLE `%s` ADD COLUMN `%s` TEXT DEFAULT('%q')", split[0], split[1], default_value);
	    }
	    case SQL_NUMBER:
	    {
	        new
				default_value
			;
			if (numargs() == 4)
			{
				default_value = getarg(3);
			}

			format(yoursql_Query, MAX_DATABASE_QUERY, "ALTER TABLE `%s` ADD COLUMN `%s` INTEGER DEFAULT('%d')", split[0], split[1], default_value);
	    }
	    case SQL_FLOAT:
	    {
	        new
				Float:default_value
			;
			if (numargs() == 4)
			{
				default_value = Float:getarg(3);
			}

			format(yoursql_Query, MAX_DATABASE_QUERY, "ALTER TABLE `%s` ADD COLUMN `%s` REAL DEFAULT('%f')", split[0], split[1], default_value);
	    }
	}
	db_free_result(db_query(yoursql_Database[id][DB_Id], yoursql_Query));

 	return true;
}

stock SQLRow:yoursql_get_row(SQL:id, const dest[], const fmat[], {_, Float}:...)
{
	if (! dest[0])
	{
	    return SQL_INVALID_ROW;
	}

	if ((numargs() - 3) * 4)
	{
	    yoursql_Query[0] = EOS;
		strcat(yoursql_Query, yoursql_returnstringarg(fmat, 3), MAX_DATABASE_QUERY);
	}
	else
	{
	    strcat(yoursql_Query, fmat, MAX_DATABASE_QUERY);
	}

	new
	    pos = strfind(yoursql_Query, "=")
	;
	if (pos <= 0)
	{
	    return SQL_INVALID_ROW;
	}

	new
	    column[MAX_DATABASE_NAME],
	    value[MAX_DATABASE_ENTRY]
	;
    yoursql_split_in_2(yoursql_Query, column, value, "=", MAX_DATABASE_NAME, MAX_DATABASE_ENTRY);

    if (! yoursql_get_db(id))
    {
        return SQL_INVALID_ROW;
    }

	new
		DBResult:result
 	;
 	format(yoursql_Query, MAX_DATABASE_QUERY, "SELECT `ROW_ID` FROM `%s` WHERE `%s` = '%q'", dest, column, value);

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
	    if (db_num_rows(result) < 1)
	    {
	        return SQL_INVALID_ROW;
	    }

		new
			rowid = db_get_field_int(result, 0)
		;

		db_free_result(result);

		return SQLRow:rowid;
	}

	return SQL_INVALID_ROW;
}

stock bool:yoursql_set_row(SQL:id, const dest[], const fmat[], {_, Float}:...)
{
	if (! dest[0])
	{
	    return false;
	}

	if ((numargs() - 3) * 4)
	{
	    yoursql_Query[0] = EOS;
		strcat(yoursql_Query, yoursql_returnstringarg(fmat, 3), MAX_DATABASE_QUERY);
	}
	else
	{
	    strcat(yoursql_Query, fmat, MAX_DATABASE_QUERY);
	}

	new
	    pos = strfind(yoursql_Query, "=")
	;
	if (pos <= 0)
	{
	    return false;
	}

	new
	    column[MAX_DATABASE_NAME],
	    value[MAX_DATABASE_ENTRY]
	;
    yoursql_split_in_2(yoursql_Query, column, value, "=", MAX_DATABASE_NAME, MAX_DATABASE_ENTRY);

    if (! yoursql_get_db(id))
    {
        return false;
    }

	new
		DBResult:   result
 	;
 	format(yoursql_Query, MAX_DATABASE_QUERY, "INSERT INTO `%s` (`%s`) VALUES('%q')", dest, column, value);

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
		db_free_result(result);

		return true;
	}

	return false;
}

stock bool:yoursql_delete_row(SQL:id, const dest[], SQLRow:rowid)
{
    if (! dest[0])
    {
		return false;
    }

    if (! yoursql_get_db(id))
    {
        return false;
    }

	new
		DBResult:result
 	;
 	format(yoursql_Query, MAX_DATABASE_QUERY, "DELETE FROM `%s` WHERE `ROW_ID` = '%i'", dest, _:rowid);

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
		db_free_result(result);

		return true;
	}

	return false;
}

stock bool:yoursql_set_field(SQL:id, const dest[], SQLRow:rowid, set[])
{
    new
        split[2][MAX_DATABASE_NAME]
	;
    yoursql_split_in_2(dest, split[0], split[1], "/", MAX_DATABASE_NAME, MAX_DATABASE_NAME);
	if (! split[0][0] || ! split[1][0])
	{
	    return false;
	}

    if (! yoursql_get_db(id))
    {
        return false;
    }

	new
		DBResult:result
 	;
 	format(yoursql_Query, MAX_DATABASE_QUERY, "UPDATE `%s` SET `%s` = '%q' WHERE `ROW_ID` = '%d'", split[0], split[1], set, _:rowid);

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
		db_free_result(result);

		return true;
	}

	return false;
}

stock bool:yoursql_set_field_int(SQL:id, const dest[], SQLRow:rowid, set)
{
    new
        split[2][MAX_DATABASE_NAME]
	;
    yoursql_split_in_2(dest, split[0], split[1], "/", MAX_DATABASE_NAME, MAX_DATABASE_NAME);
	if (! split[0][0] || ! split[1][0])
	{
	    return false;
	}

    if (! yoursql_get_db(id))
    {
        return false;
    }

	new
		DBResult:result
 	;
 	format(yoursql_Query, MAX_DATABASE_QUERY, "UPDATE `%s` SET `%s` = '%d' WHERE `ROW_ID` = '%d'", split[0], split[1], set, _:rowid);

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
		db_free_result(result);

		return true;
	}

	return false;
}

stock bool:yoursql_set_field_float(SQL:id, const dest[], SQLRow:rowid, Float:set)
{
    new
        split[2][MAX_DATABASE_NAME]
	;
    yoursql_split_in_2(dest, split[0], split[1], "/", MAX_DATABASE_NAME, MAX_DATABASE_NAME);
	if (! split[0][0] || ! split[1][0])
	{
	    return false;
	}

    if (! yoursql_get_db(id))
    {
        return false;
    }

	new
		DBResult:result
 	;
 	format(yoursql_Query, MAX_DATABASE_QUERY, "UPDATE `%s` SET `%s` = '%f' WHERE `ROW_ID` = '%d'", split[0], split[1], set, _:rowid);

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
		db_free_result(result);

		return true;
	}

	return false;
}

stock bool:yoursql_get_field(SQL:id, const dest[], SQLRow:rowid, get[], maxlength = sizeof(get))
{
    new
        split[2][MAX_DATABASE_NAME]
	;
    yoursql_split_in_2(dest, split[0], split[1], "/", MAX_DATABASE_NAME, MAX_DATABASE_NAME);
	if (! split[0][0] || ! split[1][0])
	{
	    return false;
	}

    if (! yoursql_get_db(id))
    {
        return false;
    }

	new
		DBResult:result
 	;
 	format(yoursql_Query, MAX_DATABASE_QUERY, "SELECT `%s` FROM `%s` WHERE `ROW_ID` = '%d'", split[1], split[0], _:rowid);

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
	    db_get_field(result, 0, get, maxlength);

		db_free_result(result);

		return true;
	}

	return false;
}

stock yoursql_get_field_int(SQL:id, const dest[], SQLRow:rowid)
{
    new
        split[2][MAX_DATABASE_NAME]
	;
    yoursql_split_in_2(dest, split[0], split[1], "/", MAX_DATABASE_NAME, MAX_DATABASE_NAME);
	if (! split[0][0] || ! split[1][0])
	{
	    return -1;
	}

    if (! yoursql_get_db(id))
    {
        return -1;
    }

	new
		DBResult:result
 	;
 	format(yoursql_Query, MAX_DATABASE_QUERY, "SELECT `%s` FROM `%s` WHERE `ROW_ID` = '%d'", split[1], split[0], _:rowid);

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
		new
			get = db_get_field_int(result, 0)
		;

		db_free_result(result);

		return get;
	}

	return -1;
}

stock Float:yoursql_get_field_float(SQL:id, const dest[], SQLRow:rowid)
{
    new
        split[2][MAX_DATABASE_NAME]
	;
    yoursql_split_in_2(dest, split[0], split[1], "/", MAX_DATABASE_NAME, MAX_DATABASE_NAME);
	if (! split[0][0] || ! split[1][0])
	{
	    return -1000.0;
	}

    if (! yoursql_get_db(id))
    {
        return -1000.0;
    }

	new
		DBResult:result
 	;
 	format(yoursql_Query, MAX_DATABASE_QUERY, "SELECT `%s` FROM `%s` WHERE `ROW_ID` = '%d'", split[1], split[0], _:rowid);

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
		new
			Float:get = db_get_field_float(result, 0)
		;

		db_free_result(result);

		return get;
	}

	return -1000.0;
}

stock bool:yoursql_multiset_fields(SQL:id, const dest[], SQLRow:rowid, const fmat[], {_, Float}:...)
{
    if (! dest[0])
    {
        return false;
	}

    yoursql_Query[0] = EOS;

	for (new i, j = strlen(fmat); i < j; i++)
	{
		switch (fmat[i])
		{
			case 'i', 'd', 'f', 's':
			{
			}
			default:
			{
				return false;
			}
		}
	}

	new
		column[MAX_DATABASE_NAME],
		column_arg,
		value[MAX_DATABASE_ENTRY * 2]
	;

	yoursql_Query = "UPDATE `";
	strcat(yoursql_Query, dest);
	strcat(yoursql_Query, "` SET ");

	for (new i, j = strlen(fmat); i < j; i++)
	{
		column_arg = 4 + i * 2;

		yoursql_getstringarg(column, column_arg);

		++column_arg;

		strcat(yoursql_Query, "`");
		strcat(yoursql_Query, column);
		strcat(yoursql_Query, "` = '");

		switch (fmat[i])
		{
			case 'i', 'd':
			{
				valstr(value, getarg(column_arg));
			}
			case 'f':
			{
				format(value, sizeof(value), "%f", Float:getarg(column_arg));
			}
			case 's':
			{
				yoursql_getstringarg(value, column_arg, MAX_DATABASE_ENTRY);
				value[MAX_DATABASE_ENTRY - 1] = EOS;
			}
		}

		strcat(yoursql_Query, value);

		strcat(yoursql_Query, "'");

		if (i < j - 1)
		{
			strcat(yoursql_Query, ", ");
		}
	}

	valstr(value, _:rowid);

	strcat(yoursql_Query, " WHERE `ROW_ID` = '");
	strcat(yoursql_Query, value);
	strcat(yoursql_Query, "'");

    if (! yoursql_get_db(id))
    {
        return false;
    }

	db_free_result(db_query(yoursql_Database[id][DB_Id], yoursql_Query));

	return true;
}

stock bool:yoursql_multiget_fields(SQL:id, const dest[], SQLRow:rowid, const fmat[], {_, Float}:...)
{
    if (! dest[0])
    {
        return false;
	}

	new
		column[MAX_DATABASE_NAME],
		column_arg
	;

	yoursql_Query = "SELECT `";

	for (new i, j = strlen(fmat); i < j; i++)
	{
		column_arg = 4 + i * 2;

		yoursql_getstringarg(column, column_arg);

		strcat(yoursql_Query, column);

		if (i < j - 1)
		{
			strcat(yoursql_Query, "`, `");
		}
	}

	strcat(yoursql_Query, "` FROM `");
	strcat(yoursql_Query, dest);
	strcat(yoursql_Query, "` WHERE `ROW_ID` = '");

	new
		         	str_rowid[11],
		DBResult:	result
	;

	valstr(str_rowid, _:rowid);
	strcat(yoursql_Query, str_rowid);
	strcat(yoursql_Query, "'");

    if (! yoursql_get_db(id))
    {
        return false;
    }

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
		for (new i, j = db_num_fields(result); i < j; i++)
		{
			switch (fmat[i])
			{
				case 'i', 'd':
				{
					setarg((4 + i * 2) + 1, .value = db_get_field_int(result, i));
				}
				case 'f':
				{
					setarg((4 + i * 2) + 1, .value = _:db_get_field_float(result, i));
				}
				case 's':
				{
					db_get_field(result, i, yoursql_Query, MAX_DATABASE_ENTRY);
					yoursql_setstringarg((4 + i * 2) + 1, yoursql_Query);
   				}
			}
		}

		db_free_result(result);

		return true;
	}

	return false;
}

stock yoursql_num_fields(SQL:id, const dest[])
{
    if (! yoursql_get_db(id))
    {
        return 0;
    }

    if (! dest[0])
    {
        return 0;
	}

    yoursql_Query[0] = EOS;

    new
		DBResult:result
 	;
 	strcat(yoursql_Query, "SELECT * FROM `");
	strcat(yoursql_Query, dest);
	strcat(yoursql_Query, "` WHERE 1");

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
		new
		    fields = db_num_fields(result)
		;

		db_free_result(result);

		return fields;
	}

	return 0;
}

stock yoursql_num_rows(SQL:id, const dest[])
{
    if (! yoursql_get_db(id))
    {
        return 0;
    }

    if (! dest[0])
    {
        return 0;
	}

	yoursql_Query[0] = EOS;

    new
		DBResult:result
 	;
 	strcat(yoursql_Query, "SELECT * FROM `");
	strcat(yoursql_Query, dest);
	strcat(yoursql_Query, "` WHERE 1");

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
		new
		    rows = db_num_rows(result)
		;

		db_free_result(result);

		return rows;
	}

	return 0;
}

stock yoursql_sort_int(SQL:id, const dest[], keys[], values[], limit = sizeof(keys), bool:desc = true)
{
    if (! yoursql_get_db(id))
    {
        return 0;
    }

    if (! dest[0])
    {
        return 0;
	}

    new
        split[2][MAX_DATABASE_NAME]
	;
    yoursql_split_in_2(dest, split[0], split[1], "/", MAX_DATABASE_NAME, MAX_DATABASE_NAME);
	if (! split[0][0] || ! split[1][0])
	{
	    return 0;
	}

    new
		DBResult:result
 	;
 	format(yoursql_Query, MAX_DATABASE_QUERY, "SELECT `ROW_ID`, `%s` FROM `%s` ORDER BY `%s` %s LIMIT %d", split[1], split[0], split[1], (desc == true) ? ("DESC") : ("ASC"), limit);

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
		if (db_num_rows(result) > 0)
		{
			new
				i = 0
			;
			do
			{
				keys[i] = db_get_field_int(result, 0);
				values[i] = db_get_field_int(result, 1);

				++i;
			}
			while (db_next_row(result) && i < limit);

			return i;
		}

		db_free_result(result);

		return 0;
	}

	return 0;
}

stock yoursql_sort_float(SQL:id, const dest[], keys[], Float:values[], limit = sizeof(keys), bool:desc = true)
{
    if (! yoursql_get_db(id))
    {
        return 0;
    }

    if (! dest[0])
    {
        return 0;
	}

    new
        split[2][MAX_DATABASE_NAME]
	;
    yoursql_split_in_2(dest, split[0], split[1], "/", MAX_DATABASE_NAME, MAX_DATABASE_NAME);
	if (! split[0][0] || ! split[1][0])
	{
	    return 0;
	}

    new
		DBResult:result
 	;
 	format(yoursql_Query, MAX_DATABASE_QUERY, "SELECT `ROW_ID`, `%s` FROM `%s` ORDER BY `%s` %s LIMIT %d", split[1], split[0], split[1], (desc == true) ? ("DESC") : ("ASC"), limit);

	result = db_query(yoursql_Database[id][DB_Id], yoursql_Query);
	if (result)
	{
		if (db_num_rows(result) > 0)
		{
			new
				i = 0
			;
			do
			{
				keys[i] = db_get_field_int(result, 0);
				values[i] = db_get_field_float(result, 1);

				++i;
			}
			while (db_next_row(result) && i < limit);

			return i;
		}

		db_free_result(result);

		return 0;
	}

	return 0;
}

stock DBResult:yoursql_query(SQL:id, const query[], {_, Float}:...)
{
    if (! yoursql_get_db(id))
    {
        return 0;
    }

	if ((numargs() - 3) * 4)
	{
	    yoursql_Query[0] = EOS;
		strcat(yoursql_Query, yoursql_returnstringarg(fmat, 3), MAX_DATABASE_QUERY);
	}
	else
	{
	    strcat(yoursql_Query, fmat, MAX_DATABASE_QUERY);
	}

    new
		DBResult:result = db_query(yoursql_Database[id][DB_Id], yoursql_Query)
	;

    SetTimerEx("OnDBQueryExpire", MAX_DATABASE_ALIVETIME, false, "ii", _:id, _:result);

    return result;
}

#undef MAX_DATABASE_FILES
#undef MAX_DATABASE_NAME
#undef MAX_DATABASE_ALIVETIME
#undef MAX_DATABASE_ENTRY
#undef MAX_DATABASE_QUERY
