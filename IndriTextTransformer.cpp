#include "lemur/Exception.hpp"

#include "indri/FileClassEnvironmentFactory.hpp"
#include "indri/UnparsedDocument.hpp"
#include "indri/Transformation.hpp"
#include "indri/NormalizationTransformation.hpp"
#include "indri/UTF8CaseNormalizationTransformation.hpp"
#include "indri/StemmerFactory.hpp"
#include "indri/Parameters.hpp"

void usage() {
  std::cout << "Usage: " << std::endl;
  std::cout << "    IndriTextTransformer -class=... -file=... [-stemmer=...] [-debug]" << std::endl;
  std::cout << "    1. class - TREC Format class name e.g. trectext|trecweb|... (required)" << std::endl;
  std::cout << "    2. file - TRECFormatFilePath (DO NOT NEED DOCNO, required)" << std::endl; 
  std::cout << "    3. stemmer - stemmerName porter|krovetz (optional)" << std::endl;
  std::cout << "    4. debug - debug flag (optional, 0:do not print debug info. else: print detailed info)" << std::endl;
}

void require_parameter( const char* name, indri::api::Parameters& p ) {
  if( !p.exists( name ) ) {
    usage();
    LEMUR_THROW( LEMUR_MISSING_PARAMETER_ERROR, "Must specify a " + name + " parameter." );
  }
}

int main(int argc, char * argv[]) {
  try {
    indri::api::Parameters& parameters = indri::api::Parameters::instance();
    parameters.loadCommandLine( argc, argv );

    require_parameter( "class", parameters );
    require_parameter( "file", parameters );

    indri::parse::UnparsedDocument* document;
    indri::parse::DocumentIterator* iterator;
    indri::parse::TokenizedDocument* tokenized;
    indri::api::ParsedDocument* parsed;

    std::vector<indri::parse::Transformation*> trans;
    trans.push_back( new indri::parse::NormalizationTransformation() );
    trans.push_back( new indri::parse::UTF8CaseNormalizationTransformation() );

    if ( parameters.get( "stemmer", "" ) != "") {
      if ( parameters.get( "stemmer", "" ) == "porter" || parameters.get( "stemmer", "" ) == "krovetz" ) {
        indri::api::Parameters p;
        p.set("stemmer", "");
        trans.push_back( indri::parse::StemmerFactory::get( std::string(parameters["stemmer"]), p ) );
      } else {
        std::cout << "Only support stemmer [porter] or [krovetz]" << std::endl;
        return 1;
      }
    }

    indri::parse::FileClassEnvironmentFactory _fileClassFactory;
    indri::parse::FileClassEnvironment* fce = _fileClassFactory.get( parameters["class"] );

    fce->iterator->open( parameters["file"] );

    while( document = fce->iterator->nextDocument() ) {
      //tokenized = fce->tokenizer->tokenize( raw_text );
      tokenized = fce->tokenizer->tokenize( document );
      parsed = fce->parser->parse( tokenized );
     
      for( size_t i = 0; i < trans.size(); i++ ) {
        parsed = trans[i]->transform( parsed );
      }

      for (indri::utility::greedy_vector<char*>::iterator it = parsed->terms.begin() ; it != parsed->terms.end(); ++it){
        std::cout << *it << " ";
      }
      std::cout << std::endl;

      if (parameters.get( "debug", 0 )) {
        std::cout << "-------Parsed DOCUMENT--------" << std::endl;
        std::cout << "^^^^^^^^^^^^text^^^^^^^^^^^" << std::endl;
        std::cout << tokenized->text << std::endl;
        std::cout << "^^^^^^^^^^^^text length^^^^^^^^^^^" << std::endl;
        std::cout << tokenized->textLength << std::endl;
        std::cout << "^^^^^^^^^^^^content^^^^^^^^^^^" << std::endl;
        std::cout << tokenized->content << std::endl;
        std::cout << "^^^^^^^^^^^^content length^^^^^^^^^^^" << std::endl;
        std::cout << tokenized->contentLength << std::endl;
        std::cout << tokenized->terms.size() << std::endl;
        for (indri::utility::greedy_vector<char*>::iterator it = tokenized->terms.begin() ; it != tokenized->terms.end(); ++it){
            std::cout << *it << std::endl;
        }

        std::cout << "-------Repository DOCUMENT--------" << std::endl;
        std::cout << "^^^^^^^^^^^^text^^^^^^^^^^^" << std::endl;
        std::cout << parsed->text << std::endl;
        std::cout << "^^^^^^^^^^^^text length^^^^^^^^^^^" << std::endl;
        std::cout << parsed->textLength << std::endl;
        std::cout << "^^^^^^^^^^^^content^^^^^^^^^^^" << std::endl;
        std::cout << parsed->content << std::endl;
        std::cout << "^^^^^^^^^^^^content length^^^^^^^^^^^" << std::endl;
        std::cout << parsed->contentLength << std::endl;
        std::cout << "^^^^^^^^^^^^term size^^^^^^^^^^^" << std::endl;
        std::cout << parsed->terms.size() << std::endl;
        std::cout << "^^^^^^^^^^^^position size^^^^^^^^^^^" << std::endl;
        std::cout << parsed->positions.size() << std::endl;
        std::cout << "##############################" << std::endl;
        for (indri::utility::greedy_vector<char*>::iterator it = parsed->terms.begin() ; it != parsed->terms.end(); ++it){
            std::cout << *it << std::endl;
        }
      }
    }
  } catch( lemur::api::Exception& e ) {
    LEMUR_ABORT(e);
  }

  return 0;
}