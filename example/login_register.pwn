#include <a_samp>
#include <yoursql>

//simple login and register system using YourSQL

#define DIALOG_ID_REGISTER (1)// dialogid of register dialog
#define DIALOG_ID_LOGIN (2)// dialogid of login dialog

public OnFilterScriptInit()
{
	//open the database file
	//since i am only using one database file, i won't be creating any variable with tag "SQL:" to store the db id
	//the returning values with "yoursql_open" are whole numbers so the first one would be "SQL:0"
	yoursql_open("users.db");

    //create/verify a table named "users" where we will store all the user data
    yoursql_verify_table(SQL:0, "users");

    //verify column in table "users" names "Name" to store user names
    yoursql_verify_column(SQL:0, "users/Name", SQL_STRING);
    //verify column "Password" to store user account password (SHA256 hashed)
    yoursql_verify_column(SQL:0, "users/Password", SQL_STRING);
    //verify column "Kills" to store user kills count
    yoursql_verify_column(SQL:0, "users/Kills", SQL_NUMBER);
    //verify column "Deaths" to store user deaths count
    yoursql_verify_column(SQL:0, "users/Deaths", SQL_NUMBER);
    //verify column "Score" to store user score count
    yoursql_verify_column(SQL:0, "users/Score", SQL_NUMBER);

    return 1;
}

public OnFilterScriptExit()
{
	//close the database, stored in index 0
    yoursql_close(SQL:0);

	return 1;
}

public OnPlayerConnect(playerid)
{
	new
	    name[MAX_PLAYER_NAME]
	;
	GetPlayerName(playerid, name, MAX_PLAYER_NAME);

	//check if the player is registered or not
	new
	    SQLRow:rowid = yoursql_get_row(SQL:0, "users", "Name = %s", name)
	;
	if (rowid != SQL_INVALID_ROW)//if registered
	{
	    ShowPlayerDialog(playerid, DIALOG_ID_LOGIN, DIALOG_STYLE_PASSWORD, "User Accounts:", "You are registered, please insert your password to continue:", "Login", "");
	}
	else//if new user
	{
		ShowPlayerDialog(playerid, DIALOG_ID_REGISTER, DIALOG_STYLE_PASSWORD, "User Accounts:", "You are not recognized on the database, please insert a password to sing-in and continue:", "Register", "");
	}

	return 1;
}

public OnPlayerDisconnect(playerid)
{
	//save player's score
	new
		name[MAX_PLAYER_NAME]
	;
	GetPlayerName(playerid, name, MAX_PLAYER_NAME);

	//save player score
	yoursql_set_field_int(SQL:0, "users", yoursql_get_row(SQL:0, "users", "Name = %s", name), GetPlayerScore(playerid));

	return 1;
}

public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	switch (dialogid)
	{
		case DIALOG_ID_REGISTER://if response for register dialog
		{
		    if (! response)//if the player presses 'ESC' button
		    {
		        return Kick(playerid);
		    }
		    else
		    {
		        if (! inputtext[0] || strlen(inputtext) < 4 || strlen(inputtext) > 50)//if the player's password is empty or not between 4 - 50 in length
		        {
		            SendClientMessage(playerid, 0xFF0000FF, "ERROR: Your password must be between 4 - 50 characters.");//give warning message and reshow the dialog

		            return ShowPlayerDialog(playerid, DIALOG_ID_REGISTER, DIALOG_STYLE_PASSWORD, "User Accounts:", "You are not recognized on the database, please insert a password to sing-in and continue:", "Register", "");
		        }

		        //we create the new row the same way we verufy it using "yoursql_get_row"
		        new
				    name[MAX_PLAYER_NAME]
				;
				GetPlayerName(playerid, name, MAX_PLAYER_NAME);
		        yoursql_set_row(SQL:0, "users", "Name = %s", name);//create new row with the specific "name"

		        //hash player inputtext with SHA256
		        new
		            password[128]
				;
				SHA256_PassHash(inputtext, "AfgGHne113", password, sizeof(password));
				yoursql_set_field(SQL:0, "users/password", yoursql_get_row(SQL:0, "users", "Name = %s", name), password);//set the password

		        SendClientMessage(playerid, 0x00FF00FF, "SUCCESS: You have successfully registered your account in the server.");
		    }
		}
		case DIALOG_ID_LOGIN://if response for login dialog
		{
		    if (! response)//if the player presses 'ESC' button
		    {
		        return Kick(playerid);
		    }
		    else
			{
			    //read player row and retrieve password
		        new
				    name[MAX_PLAYER_NAME]
				;
				GetPlayerName(playerid, name, MAX_PLAYER_NAME);

				//read the hashed password
				new
				    acc_password[128]
				;
				yoursql_get_field(SQL:0, "users/password", yoursql_get_row(SQL:0, "users", "Name = %s", name), acc_password);

				//read the current input password and hash it
				new
		            password[128]
				;
				SHA256_PassHash(inputtext, "AfgGHne113", password, sizeof(password));

		        if (! inputtext[0] || strcmp(password, acc_password))//if the player's password idoesn't match with the account password
		        {
		            SendClientMessage(playerid, 0xFF0000FF, "ERROR: The password doesn't match with the account's password.");//give warning message and reshow the dialog

		            return ShowPlayerDialog(playerid, DIALOG_ID_LOGIN, DIALOG_STYLE_PASSWORD, "User Accounts:", "You are registered, please insert your password to continue:", "Login", "");
		        }

		        SendClientMessage(playerid, 0x00FF00FF, "SUCCESS: You have successfully logged in your account, enjoy playing!");
		    }
		}
	}

	return 1;
}

public OnPlayerDeath(playerid, killerid, reason)
{
	new
				name[MAX_PLAYER_NAME],
		SQLRow:	rowid
	;
	
	if (killerid != INVALID_PLAYER_ID)
	{
	    //save killerid's kills
		GetPlayerName(killerid, name, MAX_PLAYER_NAME);
		
		yoursql_set_field_int(SQL:0, "users/kills", rowid, yoursql_get_field_int(SQL:0, "users/kills", rowid) + 1);//add 1 to killer kills
	}

	//save playerid's deaths
	GetPlayerName(playerid, name, MAX_PLAYER_NAME);
	
	yoursql_set_field_int(SQL:0, "users/deaths", rowid, yoursql_get_field_int(SQL:0, "users/deaths", rowid) + 1);//add 1 to player deaths

	return 1;
}
