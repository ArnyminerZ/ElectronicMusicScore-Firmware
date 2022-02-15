/**
 * @file musicxml.h
 * @author Arnau Mora (arnyminer.z@gmail.com)
 * @brief All the functions for parsing MusicXML files
 * @version 0.1
 * @date 2022-02-12
 *
 * @copyright Copyright (c) 2022
 *
 */

#ifndef MUSICXML_H
#define MUSICXML_H

// Include dependencies
#include <SPIFFS.h>
#include <fstream>
#include "mx/api/DocumentManager.h"
#include "mx/api/ScoreData.h"

// Include utils files
#include "logger.h"

#define LOAD_MUSIC_RESULT_OK 0
#define LOAD_MUSIC_RESULT_FAIL 1

int loadMusic(String path)
{
    infoln("Started parsing MusicXML at \"" + path + "\"...");
    if (!SPIFFS.exists(path)){
        errln("Could not open file at \"" + path + "\". File doesn't exist.");
        return LOAD_MUSIC_RESULT_FAIL;
        }

    debugln("Reading file...");

    // Once the XML is read, parse it
    debugln("Using MX namespace.");
    using namespace mx::api;

    // Create a reference to the singleton which holds documents in memory for us
    debugln("Getting mgr instance...");
    auto &mgr = DocumentManager::getInstance();
    debug("Opening ifstream of \"");
    debug(path);
    debugln("\"...");
    std::ifstream istr(path.c_str());

    // Ask the document manager to parse the xml into memory for us, returns a document ID.
    debugln("Creating documentId from stream...");
    const auto documentId = mgr.createFromStream(istr);

    // Get the structural representation of the score from the document manager
    debugln("Getting score data...");
    const auto score = mgr.getData(documentId);

    // We need to explicitly destroy the document from memory
    debug("Destroying document \"");
    debug(String(documentId));
    debugln("\"...");
    mgr.destroyDocument(documentId);

    if (score.parts.size() != 1)
        return LOAD_MUSIC_RESULT_FAIL;

    // drill down into the data structure to retrieve the note
    const auto &part = score.parts.at(0);
    const auto &measure = part.measures.at(0);
    const auto &staff = measure.staves.at(0);
    const auto &voice = staff.voices.at(0);
    const auto &note = voice.notes.at(0);

    if (note.durationData.durationName != DurationName::whole ||
        note.pitchData.step != Step::c)
        return LOAD_MUSIC_RESULT_FAIL;

    infoln("Finished parsing MusicXML");

    return LOAD_MUSIC_RESULT_OK;
}

#endif