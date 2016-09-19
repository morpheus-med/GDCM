// #include "arterysJSON.h"

// /*
//   "StudyInstanceUID": {
//     "Tag": "0020000D",
//     "VR": "UI",
//     "Value": [ "1.2.392.200036.9116.2.2.2.1762893313.1029997326.945873" ]
//     },
// */
// namespace arterys
// {
//     using namespace gdcm;

//     static inline bool CanContainBackslash( const VR::VRType vrtype )
//     {
//         assert( VR::IsASCII( vrtype ) );
//         // PS 3.5-2011 / Table 6.2-1 DICOM VALUE REPRESENTATIONS
//         switch( vrtype )
//             {
//             case VR::AE: // ScheduledStationAETitle
//                 //case VR::AS: // no
//                 //case VR::AT: // binary
//             case VR::CS: // SpecificCharacterSet
//             case VR::DA: // CalibrationDate
//             case VR::DS: // FrameTimeVector
//             case VR::DT: // ReferencedDateTime
//                 //case VR::FD: // binary
//                 //case VR::FL:
//             case VR::IS: // ReferencedFrameNumber
//             case VR::LO: // OtherPatientIDs
//                 //case VR::LT: // VM1
//                 //case VR::OB: // binary
//                 //case VR::OD: // binary
//                 //case VR::OF: // binary
//                 //case VR::OW: // binary
//             case VR::PN: // PerformingPhysicianName
//             case VR::SH: // PatientTelephoneNumbers
//                 //case VR::SL: // binary
//                 //case VR::SQ: // binary
//                 //case VR::SS: // binary
//                 //case VR::ST: // VM1
//             case VR::TM: // CalibrationTime
//             case VR::UI: // SOPClassesInStudy
//                 //case VR::UL: // binary
//                 //case VR::UN: // binary
//                 //case VR::US: // binary
//                 //case VR::UT: // VM1
//                 assert( !(vrtype & VR::VR_VM1) );
//                 return true;
//             default:
//                 ;
//             }
//         return false;
//     }

//     static void DataElementToJSONArray( const VR::VRType vr, const DataElement & de, const nlohmann::json & my_array )
// }
