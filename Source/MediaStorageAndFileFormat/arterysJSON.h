#ifndef ARTERYSJSON_H
#define ARTERYSJSON_H

#include "json.hpp"
#include "gdcmGlobal.h"
#include "gdcmDicts.h"
#include "gdcmBase64.h"
#include "gdcmSystem.h"

namespace arterys
{

    using namespace gdcm;

    static inline bool CanContainBackslash( const VR::VRType vrtype )
    {
        // PS 3.5-2011 / Table 6.2-1 DICOM VALUE REPRESENTATIONS
        switch( vrtype )
            {
            case VR::AE: // ScheduledStationAETitle
                //case VR::AS: // no
                //case VR::AT: // binary
            case VR::CS: // SpecificCharacterSet
            case VR::DA: // CalibrationDate
            case VR::DS: // FrameTimeVector
            case VR::DT: // ReferencedDateTime
                //case VR::FD: // binary
                //case VR::FL:
            case VR::IS: // ReferencedFrameNumber
                //case VR::LO: // OtherPatientIDs
                //case VR::LT: // VM1
                //case VR::OB: // binary
                //case VR::OD: // binary
                //case VR::OF: // binary
                //case VR::OW: // binary
            case VR::PN: // PerformingPhysicianName
            case VR::SH: // PatientTelephoneNumbers
                //case VR::SL: // binary
                //case VR::SQ: // binary
                //case VR::SS: // binary
                //case VR::ST: // VM1
            case VR::TM: // CalibrationTime
            case VR::UI: // SOPClassesInStudy
                //case VR::UL: // binary
                //case VR::UN: // binary
                //case VR::US: // binary
                //case VR::UT: // VM1
                assert( !(vrtype & VR::VR_VM1) );
                return true;
            default:
                ;
            }
        return false;
    }


    static void DataElementToJSONArray( const VR::VRType vr, const DataElement & de, nlohmann::json & top_object )
    {
        auto tag = de.GetTag().PrintAsContinuousString();
        nlohmann::json my_object = nlohmann::json({});
        if( de.IsEmpty() )
            {
                // F.2.5 DICOM JSON Model Null Values
                if( vr == VR::PN )
                    {
                        nlohmann::json my_object_comp = nlohmann::json({});
                        my_object["v"] = my_object_comp;
                    }
                top_object[tag] = my_object;
                return;
            }
        assert( !de.IsEmpty() );
        if ( !VR::IsASCII2( vr ) ) return;
        const bool checkbackslash = CanContainBackslash( vr );
        const ByteValue * bv = de.GetByteValue();

        size_t len = bv->GetLength();

        char value[len + 1];
        memcpy(value, bv->GetPointer(), len);
        value[len] = NULL;

        if( vr == VR::UI )
            {
                const std::string strui( value, len );
                my_object["v"] = strui;
            }
        else if( vr == VR::PN )
            {
                const char *str1 = value;
                // remove whitespace:
                while( str1[len-1] == ' ' )
                    {
                        len--;
                    }
                assert( str1 );
                std::stringstream ss;
                static const char *Keys[] = {
                    "Alphabetic",
                    "Ideographic",
                    "Phonetic",
                };
                nlohmann::json my_array = nlohmann::json::array();
                while (1)
                    {
                        assert( str1 && (size_t)(str1 - value) <= len );
                        const char * sep = strchr(str1, '\\');
                        const size_t llen = (sep != NULL) ? (sep - str1) : (value + len - str1);
                        const std::string component(str1, llen);

                        const char *str2 = component.c_str();
                        assert( str2 );
                        const size_t len2 = component.size();
                        assert( len2 == llen );

                        int idx = 0;
                        nlohmann::json my_object_comp;
                        while (1)
                            {
                                assert( str2 && (size_t)(str2 - component.c_str() ) <= len2 );
                                const char * sep2 = strchr(str2, '=');
                                const size_t llen2 = (sep2 != NULL) ? (sep2 - str2) : (component.c_str() + len2 - str2);
                                const std::string group(str2, llen2);
                                const char *thekey = Keys[idx++];

                                my_object_comp[thekey] = group.c_str();
                                if (sep2 == NULL) break;
                                str2 = sep2 + 1;
                            }
                        my_array.push_back(my_object_comp);
                        if (sep == NULL) break;
                        str1 = sep + 1;
                        assert( checkbackslash );
                    }
                my_object["v"] = my_array;
            }
        else if( vr == VR::DS || vr == VR::IS )
            {
                const char *str1 = value;
                assert( str1 );
                VRToType<VR::IS>::Type vris;
                VRToType<VR::DS>::Type vrds;
                nlohmann::json my_array = nlohmann::json::array();
                while (1)
                    {
                        std::stringstream ss;
                        assert( str1 && (size_t)(str1 - value) <= len );
                        const char * sep = strchr(str1, '\\');
                        size_t llen = (sep != NULL) ? (sep - str1) : (value + len - str1);
                        // This is complex, IS/DS should not be stored as string anymore
                        switch( vr )
                            {
                            case VR::IS:
                                ss.str( std::string(str1, llen) );
                                ss >> vris;
                                my_array.push_back(vris);
                                break;
                            case VR::DS:
                                ss.str( std::string(str1, llen) );
                                ss >> vrds;
                                my_array.push_back(vrds);
                                break;
                            default:
                                assert( 0 ); // programmer error
                            }
                        if (sep == NULL) break;
                        str1 = sep + 1;
                        assert( checkbackslash );
                    }
                my_object["v"] = my_array;
            }
        else if( checkbackslash )
            {
                const char *str1 = value;
                assert( str1 );
                nlohmann::json my_array = nlohmann::json::array();
                while (1)
                    {
                        assert( str1 && (size_t)(str1 - value) <= len );
                        const char * sep = strchr(str1, '\\');
                        std::stringstream ss;

                        size_t llen = (sep != NULL) ? (sep - str1) : (value + len - str1);
                        char subbuff[llen + 1];
                        memcpy(subbuff, str1, llen);
                        subbuff[llen] = NULL;
                        my_array.push_back(subbuff);
                        if (sep == NULL) break;
                        str1 = sep + 1;
                    }
                my_object["v"] = my_array;
            }
        else // default
            {
                my_object["v"] = value;
            }
        top_object[tag] = my_object;
    }
}
#endif
