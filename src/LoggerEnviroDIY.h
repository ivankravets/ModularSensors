/*
 *LoggerEnviroDIY.h
 *This file is part of the EnviroDIY modular sensors library for Arduino
 *
 *Initial library developement done by Sara Damiano (sdamiano@stroudcenter.org).
 *
 *This file is for the EnviroDIY logging functions - ie, sending JSON data to
 * http://data.enviroDIY.org
*/

#ifndef LoggerEnviroDIY_h
#define LoggerEnviroDIY_h

#include "LoggerBase.h"

// ============================================================================
//  Functions for the EnviroDIY data portal receivers.
// ============================================================================
class LoggerEnviroDIY : public Logger
{
public:
    // Set up communications
    void setToken(const char *registrationToken)
    {
        _registrationToken = registrationToken;
        DBGLOG(F("Registration token set!\n"));
    }

    void setSamplingFeature(const char *samplingFeature)
    {
        _samplingFeature = samplingFeature;
        DBGLOG(F("Sampling feature token set!\n"));
    }

    void setUUIDs(const char *UUIDs[])
    {
        _UUIDs = UUIDs;
        DBGLOG(F("UUID array set!\n"));
    }

    // This adds extra data to the datafile header
    String generateFileHeader(void)
    {
        String dataHeader = "";

        // Add additional UUID information
        String  SFHeaderString = F("Sampling Feature: ");
        SFHeaderString += _samplingFeature;
        makeHeaderRowMacro(SFHeaderString, String(_UUIDs[i]))

        // Put the basic header below
        dataHeader += Logger::generateFileHeader();

        return dataHeader;
    }

    // This generates a properly formatted JSON for EnviroDIY
    String generateSensorDataJSON(void)
    {
        String jsonString = F("{");
        jsonString += F("\"sampling_feature\": \"");
        jsonString += String(_samplingFeature) + F("\", ");
        jsonString += F("\"timestamp\": \"");
        jsonString += String(Logger::markedISO8601Time) + F("\", ");

        for (int i = 0; i < Logger::_variableCount; i++)
        {
            jsonString += F("\"");
            jsonString += String(_UUIDs[i]) + F("\": ");
            jsonString += Logger::_variableList[i]->getValueString();
            if (i + 1 != Logger::_variableCount)
            {
                jsonString += F(", ");
            }
        }

        jsonString += F(" }");
        return jsonString;
    }

    // Communication functions
    void streamEnviroDIYRequest(Stream *stream)
    {
        stream->print(String(F("POST /api/data-stream/ HTTP/1.1")));
        stream->print(String(F("\r\nHost: data.envirodiy.org")));
        stream->print(String(F("\r\nTOKEN: ")) + String(_registrationToken));
        // stream->print(String(F("\r\nCache-Control: no-cache")));
        // stream->print(String(F("\r\nConnection: close")));
        stream->print(String(F("\r\nContent-Length: ")) + String(generateSensorDataJSON().length()));
        stream->print(String(F("\r\nContent-Type: application/json\r\n\r\n")));
        stream->print(String(generateSensorDataJSON()));
    }


    // Public function to send data
    int postDataEnviroDIY(void)
    {
        // Create a buffer for the response
        char response_buffer[12] = "";
        int did_respond = 0;

        // Open a TCP/IP connection to the Enviro DIY Data Portal (WebSDL)
        if(modem.connect("data.envirodiy.org", 80))
        {
            // Send the request to the serial for debugging
            #if defined(MODULAR_SENSORS_OUTPUT)
                PRINTOUT(F("\n \\/---- Post Request to EnviroDIY ----\\/ \n"));
                streamEnviroDIYRequest(&MODULAR_SENSORS_OUTPUT);  // for debugging
                PRINTOUT(F("\r\n\r\n"));
                MODULAR_SENSORS_OUTPUT.flush();  // for debugging
            #endif

            // Send the request to the modem stream
            modem.dumpBuffer(modem._client);
            streamEnviroDIYRequest(modem._client);
            modem._client->flush();  // wait for sending to finish

            uint32_t start_timer;
            if (millis() < 4294957296) start_timer = millis();  // In case of roll-over
            else start_timer = 0;
            while ((millis() - start_timer) < 10000L && modem._client->available() < 12)
            {delay(10);}

            // Read only the first 12 characters of the response
            // We're only reading as far as the http code, anything beyond that
            // we don't care about so we're not reading to save on total
            // data used for transmission.
            did_respond = modem._client->readBytes(response_buffer, 12);

            // Close the TCP/IP connection as soon as the first 12 characters are read
            // We don't need anything else and stoping here should save data use.
            modem.stop();
        }
        else PRINTOUT(F("\n -- Unable to Establish Connection to EnviroDIY Data Portal -- \n"));

        // Process the HTTP response
        int responseCode = 0;
        if (did_respond > 0)
        {
            char responseCode_char[4];
            for (int i = 0; i < 3; i++)
            {
                responseCode_char[i] = response_buffer[i+9];
            }
            responseCode = atoi(responseCode_char);
            // modem.dumpBuffer(modem._client);
        }
        else responseCode=504;

        PRINTOUT(F(" -- Response Code -- \n"));
        PRINTOUT(responseCode, F("\n"));

        return responseCode;
    }

    // ===================================================================== //
    // Convience functions to call several of the above functions
    // ===================================================================== //

    // This is a one-and-done to log data
    virtual void log(void) override
    {
        // Check of the current time is an even interval of the logging interval
        if (checkInterval())
        {
            // Print a line to show new reading
            PRINTOUT(F("------------------------------------------\n"));
            // Turn on the LED to show we're taking a reading
            digitalWrite(_ledPin, HIGH);

            // Turn on the modem to let it start searching for the network
            modem.wake();

            // Wake up all of the sensors
            // I'm not doing as part of sleep b/c it may take up to a second or
            // two for them all to wake which throws off the checkInterval()
            sensorsWake();
            // Update the values from all attached sensors
            updateAllSensors();
            // Immediately put sensors to sleep to save power
            sensorsSleep();

            // Connect to the network
            if (modem.connectNetwork())
            {
                // Post the data to the WebSDL
                postDataEnviroDIY();

                // Sync the clock every 288 readings (1/day at 5 min intervals)
                if (_numReadings % 288 == 0)
                {
                    syncRTClock();
                }

                // Disconnect from the network
                modem.disconnectNetwork();
            }

            // Turn the modem off
            modem.off();

            // Create a csv data record and save it to the log file
            logToSD(generateSensorDataCSV());

            // Turn off the LED
            digitalWrite(_ledPin, LOW);
            // Print a line to show reading ended
            PRINTOUT(F("------------------------------------------\n\n"));
        }

        // Sleep
        if(_sleep){systemSleep();}
    }


private:
    // Tokens and UUID's for EnviroDIY
    const char *_registrationToken;
    const char *_samplingFeature;
    const char **_UUIDs;
};

#endif
