package com.example.homeos

import android.os.Bundle
import androidx.activity.ComponentActivity
import androidx.activity.compose.setContent
import androidx.compose.foundation.layout.*
import androidx.compose.material3.*
import androidx.compose.runtime.*
import androidx.compose.ui.Alignment
import androidx.compose.ui.Modifier
import androidx.compose.ui.unit.dp
import kotlinx.coroutines.Dispatchers
import kotlinx.coroutines.launch
import java.net.HttpURLConnection
import java.net.URL


class MainActivity : ComponentActivity() {


    override fun onCreate(savedInstanceState: Bundle?) {
        super.onCreate(savedInstanceState)


        setContent {

            HomeOSApp()

        }
    }
}


// ===============================
// HOMEOS APP UI
// ===============================


@Composable
fun HomeOSApp()
{

    val scope = rememberCoroutineScope()


    // ESP32 STATIC IP

    val espIP =
        "http://10.249.244.50"



    Surface(
        modifier = Modifier.fillMaxSize()
    )
    {


        Column(

            modifier =
            Modifier
                .fillMaxSize()
                .padding(20.dp),

            horizontalAlignment =
            Alignment.CenterHorizontally,

            verticalArrangement =
            Arrangement.Center

        )
        {


            Text(
                text = "HomeOS Controller",
                style =
                MaterialTheme.typography.headlineMedium
            )


            Spacer(
                modifier =
                Modifier.height(30.dp)
            )



            for(i in 1..6)
            {


                Row(

                    verticalAlignment =
                    Alignment.CenterVertically

                )
                {



                    Button(

                        onClick =
                        {

                            scope.launch(
                                Dispatchers.IO
                            )
                            {

                                sendCommand(
                                    "$espIP/r${i}on"
                                )

                            }

                        }

                    )
                    {

                        Text(
                            "Relay $i ON"
                        )

                    }



                    Spacer(
                        modifier =
                        Modifier.width(15.dp)
                    )



                    Button(

                        onClick =
                        {

                            scope.launch(
                                Dispatchers.IO
                            )
                            {

                                sendCommand(
                                    "$espIP/r${i}off"
                                )

                            }

                        }

                    )
                    {

                        Text(
                            "OFF"
                        )

                    }


                }


                Spacer(
                    modifier =
                    Modifier.height(10.dp)
                )


            }


        }


    }


}



// ===============================
// HTTP COMMUNICATION
// ===============================


fun sendCommand(
    link:String
)
{

    try
    {

        val url =
            URL(link)


        val connection =
            url.openConnection()
                    as HttpURLConnection


        connection.requestMethod =
            "GET"


        connection.connectTimeout =
            3000


        connection.readTimeout =
            3000


        connection.connect()


        connection.inputStream
            .bufferedReader()
            .readText()


        connection.disconnect()


    }


    catch(e:Exception)
    {

        e.printStackTrace()

    }


}
