#include <yoursql>

#pragma dynamic (10000)

new SQL:g_Database;

public OnGameModeInit()
{
	g_Database = yoursql_open("users.db");
	yoursql_verify_table(g_Database, "users");
	yoursql_verify_column(g_Database, "users/name", SQL_STRING);

	strcat("", "", 0);
	strunpack("", "", 0);

    #if defined udb_OnGameModeInit
       	return udb_OnGameModeInit();
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnGameModeInit
    #undef OnGameModeInit
#else
    #define _ALS_OnGameModeInit
#endif
#define OnGameModeInit udb_OnGameModeInit
#if defined udb_OnGameModeInit
    forward udb_OnGameModeInit();
#endif

public OnFilterScriptInit()
{
	if (g_Database == SQL_INVALID_DB)
	{
		g_Database = yoursql_open("users.db");
		yoursql_verify_table(g_Database, "users");
		yoursql_verify_column(g_Database, "users/name", SQL_STRING);

		strcat("", "", 0);
		strunpack("", "", 0);
	}

    #if defined udb_OnFilterScriptInit
       	return udb_OnFilterScriptInit();
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnFilterScriptInit
    #undef OnFilterScriptInit
#else
    #define _ALS_OnFilterScriptInit
#endif
#define OnFilterScriptInit udb_OnFilterScriptInit
#if defined udb_OnFilterScriptInit
    forward udb_OnFilterScriptInit();
#endif

public OnGameModeExit()
{
    yoursql_close(g_Database);

    #if defined udb_OnGameModeExit
       	return udb_OnGameModeExit();
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnGameModeExit
    #undef OnGameModeExit
#else
    #define _ALS_OnGameModeExit
#endif
#define OnGameModeExit udb_OnGameModeExit
#if defined udb_OnGameModeExit
    forward udb_OnGameModeExit();
#endif

public OnFilterScriptExit()
{
	if (g_Database != SQL_INVALID_DB)
	{
   	 	yoursql_close(g_Database);
	}

    #if defined udb_OnFilterScriptExit
       	return udb_OnFilterScriptExit();
	#else
	   	return 1;
	#endif
}
#if defined _ALS_OnFilterScriptExit
    #undef OnFilterScriptExit
#else
    #define _ALS_OnFilterScriptExit
#endif
#define OnFilterScriptExit udb_OnFilterScriptExit
#if defined udb_OnFilterScriptExit
    forward udb_OnFilterScriptExit();
#endif

stock udb_add(const field[], SQLType:type)
{
	new dest[100];
	strcat(dest, "users/");
	strcat(dest, field);

	yoursql_verify_column(g_Database, dest, type);
}

stock udb_set_int(playerid = -1, name[] = "", const field[], set)
{
	if (! name[0])
	{
	    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
	}

	new dest[100];
	strcat(dest, "users/");
	strcat(dest, field);
	
	new SQLRow:rowid = yoursql_get_row(g_Database, "users", "name = %s", name);

	if (rowid == SQL_INVALID_ROW)
	{
	    if (! yoursql_multiset_row(g_Database, "users", "si", "name", name, field, set))
	    {
	        yoursql_verify_column(g_Database, dest, SQL_NUMBER);
	    }
	    
	    yoursql_multiset_row(g_Database, "users", "si", "name", name, field, set);
	    return;
	}

	if (! yoursql_set_field_int(g_Database, dest, rowid, set))
	{
	    yoursql_verify_column(g_Database, dest, SQL_NUMBER);
	}
	yoursql_set_field_int(g_Database, dest, rowid, set);
}

stock udb_set_float(playerid = -1, name[] = "", const field[], Float:set)
{
	if (! name[0])
	{
	    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
	}

	new dest[100];
	strcat(dest, "users/");
	strcat(dest, field);

	new SQLRow:rowid = yoursql_get_row(g_Database, "users", "name = %s", name);

	if (rowid == SQL_INVALID_ROW)
	{
	    if (! yoursql_multiset_row(g_Database, "users", "sf", "name", name, field, set))
	    {
	        yoursql_verify_column(g_Database, dest, SQL_FLOAT);
	    }
	    
	    yoursql_multiset_row(g_Database, "users", "sf", "name", name, field, set);
	    return;
	}

	if (! yoursql_set_field_float(g_Database, dest, rowid, set))
	{
	    yoursql_verify_column(g_Database, dest, SQL_FLOAT);
	}
	yoursql_set_field_float(g_Database, dest, rowid, set);
}

stock udb_set(playerid = -1, name[] = "", const field[], set[])
{
	if (! name[0])
	{
	    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
	}

	new dest[100];
	strcat(dest, "users/");
	strcat(dest, field);

	new SQLRow:rowid = yoursql_get_row(g_Database, "users", "name = %s", name);

	if (rowid == SQL_INVALID_ROW)
	{
	    if (! yoursql_multiset_row(g_Database, "users", "ss", "name", name, field, set))
	    {
	        yoursql_verify_column(g_Database, dest, SQL_STRING);
	    }

	    yoursql_multiset_row(g_Database, "users", "ss", "name", name, field, set);
	    return;
	}

	if (! yoursql_set_field(g_Database, dest, rowid, set))
	{
	    yoursql_verify_column(g_Database, dest, SQL_STRING);
	}
	yoursql_set_field(g_Database, dest, rowid, set);
}

stock udb_get_int(playerid = -1, name[] = "", const field[])
{
	if (! name[0])
	{
	    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
	}

	new dest[100];
	strcat(dest, "users/");
	strcat(dest, field);
	return yoursql_get_field_int(g_Database, dest, yoursql_get_row(g_Database, "users", "name = %s", name));
}

stock udb_get_float(playerid = -1, name[] = "", const field[])
{
	if (! name[0])
	{
	    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
	}

	new dest[100];
	strcat(dest, "users/");
	strcat(dest, field);
	return yoursql_get_field_float(g_Database, dest, yoursql_get_row(g_Database, "users", "name = %s", name));
}

stock udb_get(playerid = -1, name[] = "", const field[], get[], size = sizeof(get))
{
	if (! name[0])
	{
	    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
	}

	new dest[100];
	strcat(dest, "users/");
	strcat(dest, field);
	return yoursql_get_field(g_Database, dest, yoursql_get_row(g_Database, "users", "name = %s", name), get, size);
}

stock udb_remove(playerid = -1, name[] = "")
{
	if (! name[0])
	{
	    GetPlayerName(playerid, name, MAX_PLAYER_NAME);
	}

	yoursql_delete_row(g_Database, "users", yoursql_get_row(g_Database, "users", "name = %s", name));
}
