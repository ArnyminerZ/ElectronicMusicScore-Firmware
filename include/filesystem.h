/**
 * @file filesystem.h
 * @author Arnau Mora (arnyminer.z@gmail.com)
 * @brief File system related functions
 * @version 0.1
 * @date 2022-02-09
 * 
 * @copyright Copyright (c) 2022
 * 
 */

#ifndef FILESYSTEM_H
#define FILESYSTEM_H

// Include libraries
#include <Arduino.h>
#include <SPIFFS.h>

// Utilities
#include "utils.h"

// function defaults
String listFiles(bool ishtml = false);

// list all of the files, if ishtml=true, return html rather than simple text
/**
 * @brief Lists all the files in the SPIFFS.
 * 
 * @param backend If true, the result will be in JSON, otherwise, the result will be in a "readable" format.
 * @return String The list of files in the SPIFFS.
 */
String listFiles(bool backend)
{
  String returnText = "";
  Serial.println("Listing files stored on SPIFFS");
  File root = SPIFFS.open("/");
  File foundfile = root.openNextFile();
  if (backend)
    returnText += "{\"files\":[";
  while (foundfile)
  {
    String filename = String(foundfile.name());
    size_t filesize = foundfile.size();
    foundfile = root.openNextFile();
    if (backend)
      returnText += "{\"name\":\"" + filename + "\",\"size\":\"" + String(filesize) + "\"}";
    else
      returnText += "File: " + filename + " Size: " + humanReadableSize(filesize) + "\n";
      
    if (backend && foundfile)
      returnText += ",";
  }
  if (backend)
    returnText = returnText + "]}";

  root.close();
  foundfile.close();
  return returnText;
}

#endif