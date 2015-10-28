/*
* ================
* SAMP - YourSQL
* The easiest and one of the fastest SQLITE manager, library. Requires you with no SQL knowledge!
* ================
*
* ================
* Create by: Gammix
* ================
*
* ================
* Special thanks to: Slice
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
	#define SQL_INVALID_ROW (SQLRow:-1)
#endif

public OnFilterScriptInit()
{
	new
	    __str[5]
 	;
	format(__str, sizeof(__str), "");
	strcat(__str, "");
	
	#pragma unused __str

	#if defined yoursql_OnFilterScriptInit
		return yoursql_OnFilterScriptInit();
	#else
		return 1;
	#endif
}
#if defined _ALS_OnFilterScriptInit
	#undef OnFilterScriptInit
#else
	#define _ALS_OnFilterScriptInit
#endif
#define OnFilterScriptInit yoursql_OnFilterScriptInit
#if defined yoursql_OnFilterScriptInit
	forward yoursql_OnFilterScriptInit();
#endif

#if ! defined yoursql_OnFilterScriptInit
	public OnGameModeInit()
	{
		new
		    __str[5]
	 	;
		format(__str, sizeof(__str), "");
		strcat(__str, "");

		#pragma unused __str

		#if defined yoursql_OnGameModeInit
			return yoursql_OnGameModeInit();
		#else
			return 1;
		#endif
	}
	#if defined _ALS_OnGameModeInit
		#undef OnGameModeInit
	#else
		#define _ALS_OnGameModeInit
	#endif
	#define OnGameModeInit yoursql_OnGameModeInit
	#if defined yoursql_OnGameModeInit
		forward yoursql_OnGameModeInit();
	#endif
#endif

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

stock static yoursql_str_split(const src[], dest[][], delimeter, limit, size)
{
	new
	    k,
	    l
	;
	for (new i = 0, j = strlen(src); i <= j; i++)
	{
		if (src[i] == delimeter || i == j)
		{
			new
			    m = strmid(dest[k], src, l, i, size)
			;

   			dest[k][m] = 0;

   			l = i + 1;
   			k += 1;

   			if (k >= limit)
   			{
				return k;
   			}
		}
	}

	return k;
}

stock static __getstringarg(dest[], arg, len = sizeof (dest))
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

stock static __setstringarg(arg, const value[], len = sizeof(value))
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
	yoursql_str_split(dest, split, '/', 2, MAX_DATABASE_NAME);
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
				__getstringarg(default_value, 3);
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

	static const
		STATIC_ARGS = 3,
		BYTES_PER_CELL = 4
	;
	
    new
		n = (numargs() - STATIC_ARGS) * BYTES_PER_CELL
	;

	const
		output_size = MAX_DATABASE_QUERY
	;
	
	yoursql_Query[0] = EOS;
	
    if (n)
    {
        new
			arg_start,
			arg_end
		;
		
        #emit CONST.alt        	fmat
        #emit LCTRL          	5
        #emit ADD
        #emit STOR.S.pri        arg_start

        #emit LOAD.S.alt        n
        #emit ADD
        #emit STOR.S.pri        arg_end
        do
        {
            #emit LOAD.I
            #emit PUSH.pri
            arg_end -= BYTES_PER_CELL;
            #emit LOAD.S.pri	arg_end
        }
        while (arg_end > arg_start);

        #emit PUSH.S          	fmat
        #emit PUSH.C           	output_size
        #emit PUSH.ADR         	yoursql_Query

        n += BYTES_PER_CELL * 3;
        #emit PUSH.S          	n
        #emit SYSREQ.C          format

        n += BYTES_PER_CELL;
        #emit LCTRL          	4
        #emit LOAD.S.alt        n
        #emit ADD
        #emit SCTRL          	4
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
	strmid(column, yoursql_Query, 0, (yoursql_Query[pos - 1] == ' ') ? (pos - 1) : (pos), MAX_DATABASE_NAME);
	strmid(value, yoursql_Query, (yoursql_Query[pos + 1] == ' ') ? (pos + 2) : (pos + 1), strlen(yoursql_Query), MAX_DATABASE_ENTRY);

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

	static const
		STATIC_ARGS = 3,
		BYTES_PER_CELL = 4
	;
	
    new
		n = (numargs() - STATIC_ARGS) * BYTES_PER_CELL
	;

	const
		output_size = MAX_DATABASE_QUERY
	;
	
	yoursql_Query[0] = EOS;

    if (n)
    {
        new
			arg_start,
			arg_end
		;

        #emit CONST.alt        	fmat
        #emit LCTRL          	5
        #emit ADD
        #emit STOR.S.pri        arg_start

        #emit LOAD.S.alt        n
        #emit ADD
        #emit STOR.S.pri        arg_end
        do
        {
            #emit LOAD.I
            #emit PUSH.pri
            arg_end -= BYTES_PER_CELL;
            #emit LOAD.S.pri	arg_end
        }
        while (arg_end > arg_start);

        #emit PUSH.S          	fmat
        #emit PUSH.C          	output_size
        #emit PUSH.ADR         	yoursql_Query

        n += BYTES_PER_CELL * 3;
        #emit PUSH.S          	n
        #emit SYSREQ.C          format

        n += BYTES_PER_CELL;
        #emit LCTRL          	4
        #emit LOAD.S.alt        n
        #emit ADD
        #emit SCTRL          	4
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
	strmid(column, yoursql_Query, 0, (yoursql_Query[pos - 1] == ' ') ? (pos - 1) : (pos), MAX_DATABASE_NAME);
	strmid(value, yoursql_Query, (yoursql_Query[pos + 1] == ' ') ? (pos + 2) : (pos + 1), strlen(yoursql_Query), MAX_DATABASE_ENTRY);

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
	yoursql_str_split(dest, split, '/', 2, MAX_DATABASE_NAME);
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
	yoursql_str_split(dest, split, '/', 2, MAX_DATABASE_NAME);
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
	yoursql_str_split(dest, split, '/', 2, MAX_DATABASE_NAME);
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
	yoursql_str_split(dest, split, '/', 2, MAX_DATABASE_NAME);
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
	yoursql_str_split(dest, split, '/', 2, MAX_DATABASE_NAME);
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
	yoursql_str_split(dest, split, '/', 2, MAX_DATABASE_NAME);
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

		__getstringarg(column, column_arg);

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
				__getstringarg(value, column_arg, MAX_DATABASE_ENTRY);
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

		__getstringarg(column, column_arg);

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
					__setstringarg((4 + i * 2) + 1, yoursql_Query);
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
	yoursql_str_split(dest, split, '/', 2, MAX_DATABASE_NAME);
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
	yoursql_str_split(dest, split, '/', 2, MAX_DATABASE_NAME);
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

	static const
		STATIC_ARGS = 3,
		BYTES_PER_CELL = 4
	;

    new
		n = (numargs() - STATIC_ARGS) * BYTES_PER_CELL,
	;
	
	const
	    output_size = MAX_DATABASE_QUERY
	;
	
	yoursql_Query[0] = EOS;

    if (n)
    {
        new
			arg_start,
			arg_end
		;

        #emit CONST.alt        	fmat
        #emit LCTRL          	5
        #emit ADD
        #emit STOR.S.pri        arg_start

        #emit LOAD.S.alt        n
        #emit ADD
        #emit STOR.S.pri        arg_end
        do
        {
            #emit LOAD.I
            #emit PUSH.pri
            arg_end -= BYTES_PER_CELL;
            #emit LOAD.S.pri	arg_end
        }
        while (arg_end > arg_start);

        #emit PUSH.S          	fmat
        #emit PUSH.C          	output_size
        #emit PUSH.ADR         	yoursql_Query

        n += BYTES_PER_CELL * 3;
        #emit PUSH.S          	n
        #emit SYSREQ.C          format

        n += BYTES_PER_CELL;
        #emit LCTRL          	4
        #emit LOAD.S.alt        n
        #emit ADD
        #emit SCTRL          	4
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
