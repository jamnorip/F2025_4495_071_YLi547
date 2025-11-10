<?php

require_once 'includes/session.php';

class LoginManager  {

    //This function checks if the user is logged in, if they are not they are redirected to the login page
    static function verifyLogin()   {

        //Check for a session
        // if (session_id() == '' || !isset($_SESSION)) {

        //     //Start it up
        //     // session_start();
        // }
        //Is anyone logged in.
        if (isset($_SESSION["username"]))  {

            //The user is loggedin
            return true;

        } else {

            //The user is not logged in
            //Destroy any session just in case
            session_destroy();
                   
            return false;

        }
    }
        
    
}

?>