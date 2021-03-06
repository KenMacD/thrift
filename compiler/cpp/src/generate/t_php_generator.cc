/*
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements. See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License. You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied. See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include <stdlib.h>
#include <sys/stat.h>
#include <sstream>
#include "t_oop_generator.h"
#include "platform.h"
using namespace std;

#define NSGLOBAL  (nsglobal_.size() ? nsglobal_ : "")
#define NSGLOBAL_A ("\\" + NSGLOBAL )
#define NSGLOBAL_B ( NSGLOBAL + "\\")
#define NSGLOBAL_AB ("\\" + NSGLOBAL + "\\")
#define NS_ROOT ( namespace_php_ ? "\\" : "")


/**
 * PHP code generator.
 *
 */
class t_php_generator : public t_oop_generator {
 public:
  t_php_generator(
      t_program* program,
      const std::map<std::string, std::string>& parsed_options,
      const std::string& option_string)
    : t_oop_generator(program)
  {
    (void) option_string;
    std::map<std::string, std::string>::const_iterator iter;

    iter = parsed_options.find("inlined");
    binary_inline_ = (iter != parsed_options.end());

    iter = parsed_options.find("rest");
    rest_ = (iter != parsed_options.end());

    iter = parsed_options.find("server");
    phps_ = (iter != parsed_options.end());

    iter = parsed_options.find("autoload");
    autoload_ = (iter != parsed_options.end());

    iter = parsed_options.find("oop");
    oop_ = (iter != parsed_options.end());

    iter = parsed_options.find("namespace");
    namespace_php_ = (iter != parsed_options.end());

    iter = parsed_options.find("nsglobal");
    if(iter != parsed_options.end()) {
      if(namespace_php_) nsglobal_ = iter->second;
      else throw "cannot use nsglobal without namespace.";
    }
    else {
      nsglobal_ = ""; // by default global namespace is empty
    }

    if (oop_ && binary_inline_) {
      throw "oop and inlined are mutually exclusive.";
    }

    out_dir_base_ = (binary_inline_ ? "gen-phpi" : "gen-php");
    escape_['$'] = "\\$";
  }

  static bool is_valid_namespace(const std::string& sub_namespace);

  /**
   * Init and close methods
   */

  void init_generator();
  void close_generator();

  /**
   * Program-level generation functions
   */

  void generate_typedef  (t_typedef*  ttypedef);
  void generate_enum     (t_enum*     tenum);
  void generate_const    (t_const*    tconst);
  void generate_struct   (t_struct*   tstruct);
  void generate_xception (t_struct*   txception);
  void generate_service  (t_service*  tservice);

  std::string render_const_value(t_type* type, t_const_value* value);

  /**
   * Structs!
   */

  void generate_php_struct(t_struct* tstruct, bool is_exception);
  void generate_php_struct_definition(std::ofstream& out, t_struct* tstruct, bool is_xception=false);
  void _generate_php_struct_definition(std::ofstream& out, t_struct* tstruct, bool is_xception=false);
  void generate_php_struct_reader(std::ofstream& out, t_struct* tstruct);
  void generate_php_struct_writer(std::ofstream& out, t_struct* tstruct);
  void generate_php_function_helpers(t_function* tfunction);

  void generate_php_type_spec(std::ofstream &out, t_type* t);
  void generate_php_struct_spec(std::ofstream &out, t_struct* tstruct);

  /**
   * Service-level generation functions
   */

  void generate_service_helpers   (t_service* tservice);
  void generate_service_interface (t_service* tservice);
  void generate_service_rest      (t_service* tservice);
  void generate_service_client    (t_service* tservice);
  void _generate_service_client   (std::ofstream &out, t_service* tservice);
  void generate_service_processor (t_service* tservice);
  void generate_process_function  (t_service* tservice, t_function* tfunction);

  /**
   * Serialization constructs
   */

  void generate_deserialize_field        (std::ofstream &out,
                                          t_field*    tfield,
                                          std::string prefix="",
                                          bool inclass=false);

  void generate_deserialize_struct       (std::ofstream &out,
                                          t_struct*   tstruct,
                                          std::string prefix="");

  void generate_deserialize_container    (std::ofstream &out,
                                          t_type*     ttype,
                                          std::string prefix="");

  void generate_deserialize_set_element  (std::ofstream &out,
                                          t_set*      tset,
                                          std::string prefix="");

  void generate_deserialize_map_element  (std::ofstream &out,
                                          t_map*      tmap,
                                          std::string prefix="");

  void generate_deserialize_list_element (std::ofstream &out,
                                          t_list*     tlist,
                                          std::string prefix="");

  void generate_serialize_field          (std::ofstream &out,
                                          t_field*    tfield,
                                          std::string prefix="");

  void generate_serialize_struct         (std::ofstream &out,
                                          t_struct*   tstruct,
                                          std::string prefix="");

  void generate_serialize_container      (std::ofstream &out,
                                          t_type*     ttype,
                                          std::string prefix="");

  void generate_serialize_map_element    (std::ofstream &out,
                                          t_map*      tmap,
                                          std::string kiter,
                                          std::string viter);

  void generate_serialize_set_element    (std::ofstream &out,
                                          t_set*      tmap,
                                          std::string iter);

  void generate_serialize_list_element   (std::ofstream &out,
                                          t_list*     tlist,
                                          std::string iter);

  /**
   * Helper rendering functions
   */

  std::string php_includes();
  std::string declare_field(t_field* tfield, bool init=false, bool obj=false);
  std::string function_signature(t_function* tfunction, std::string prefix="");
  std::string argument_list(t_struct* tstruct);
  std::string type_to_cast(t_type* ttype);
  std::string type_to_enum(t_type* ttype);

  std::string php_namespace_base(const t_program* p, bool modern) {
    std::string ns = p->get_namespace("php");
    const char * delimiter = modern ? "\\" : "_";
    size_t position = ns.find('.');
    while (position != string::npos) {
      ns.replace(position, 1, delimiter);
      position = ns.find('.', position + 1);
    }
    return ns;
  }

  //general use namespace prefixing: \my\namespace\ or my_namespace_
  string php_namespace(const t_program* p) {
    string ns = php_namespace_base(p, namespace_php_);
    if(namespace_php_) return (nsglobal_.size() ? NSGLOBAL_AB : NSGLOBAL_B) + (ns.size() ? (ns + "\\") : "");
    else return (nsglobal_.size() ? NSGLOBAL + "_" : "") + (ns.size() ? (ns + "_") : "");
  }

  //setting the namespace of a file: my\namespace
  string php_namespace_suffix(const t_program* p) {
    string ns = php_namespace_base(p, namespace_php_);
    if (namespace_php_) return (nsglobal_.size() ? NSGLOBAL_B : NSGLOBAL) + ns;
    //provides consistent behavior even though this function is never called when namespace_php_ is false
    else return (nsglobal_.size() ? (NSGLOBAL + "_") : "") + ns;
  }

  //writing an autload identifier into globals: my\namespace\ or my_namespace_
  string php_namespace_autoload(const t_program* p) {
    std::string ns = php_namespace_base(p, namespace_php_);
    if (namespace_php_) return (nsglobal_.size() ? NSGLOBAL_B : NSGLOBAL) + (ns.size() ? (ns + "\\") : "");
    else return (nsglobal_.size() ? (NSGLOBAL + "_") : "") + (ns.size() ? (ns + "_") : "");
  }

  string php_namespace_constant(const t_program* p) {
    std::string ns = php_namespace_base(p, false);
    return ns.size() ? (ns + "_") : "";
  }

  //declaring a type: typename or my_namespace_typename
  string php_namespace_declaration(t_type* t) {
    if(namespace_php_) return t->get_name();
    return php_namespace(t->get_program()) + t->get_name();
  }

  std::string php_path(t_program* p) {
    std::string ns = p->get_namespace("php.path");
    if (ns.empty()) {
      return p->get_name();
    }

    // Transform the java-style namespace into a path.
    for (std::string::iterator it = ns.begin(); it != ns.end(); ++it) {
      if (*it == '.') {
        *it = '/';
      }
    }

    return ((namespace_php_) ? ns + '/' : "" ) + p->get_name();
  }

 private:

  /**
   * File streams
   */
  std::ofstream f_types_;
  std::ofstream f_consts_;
  std::ofstream f_helpers_;
  std::ofstream f_service_;

  std::string package_dir_;
  /**
   * Generate protocol-independent template? Or Binary inline code?
   */
  bool binary_inline_;

  /**
   * Generate a REST handler class
   */
  bool rest_;

  /**
   * Generate stubs for a PHP server
   */
  bool phps_;

  /**
   * Generate PHP code that uses autoload
   */
  bool autoload_;

  /**
   * Whether to use OOP base class TBase
   */
  bool oop_;
  
  /**
   * Generate namespaces in PHP5.3 style
   */
  bool namespace_php_;
 
  /**
   * Global namespace for PHP 5.3
   */
  std::string nsglobal_;
};


bool t_php_generator::is_valid_namespace(const std::string& sub_namespace) {
  return sub_namespace == "path";
}


/**
 * Prepares for file generation by opening up the necessary file output
 * streams.
 *
 * @param tprogram The program to generate
 */
void t_php_generator::init_generator() {
  // Make output directory
  MKDIR(get_out_dir().c_str());
  package_dir_ = get_out_dir()+"/"+program_name_+"/";
  MKDIR(package_dir_.c_str());
  // Make output file
  string f_types_name = package_dir_+program_name_+"_types.php";
  f_types_.open(f_types_name.c_str());

  // Print header
  f_types_ <<
    "<?php" << endl;
  if(namespace_php_) f_types_ << "namespace " << php_namespace_suffix(get_program()) << ";" << endl;
  f_types_ << autogen_comment() << php_includes();

  // Include other Thrift includes
  const vector<t_program*>& includes = program_->get_includes();
  for (size_t i = 0; i < includes.size(); ++i) {
    string package = includes[i]->get_name();
    string prefix = php_path(includes[i]);
    f_types_ <<
      "include_once $GLOBALS['THRIFT_ROOT'].'/packages/" << prefix << "/" << package << "_types.php';" << endl;
  }
  f_types_ << endl;

  // Print header
  if (!program_->get_consts().empty()) {
    string f_consts_name = package_dir_+program_name_+"_constants.php";
    f_consts_.open(f_consts_name.c_str());
    f_consts_ <<
      "<?php" << endl;
     if(namespace_php_) f_consts_ << "namespace " << php_namespace_suffix(get_program()) << ";" << endl;
     f_consts_ << autogen_comment() <<
      "include_once $GLOBALS['THRIFT_ROOT'].'/packages/" + php_path(program_) + "/" + program_name_ + "_types.php';" << endl <<
      endl <<
      "$GLOBALS['" << php_namespace_constant(get_program()) << program_name_ << "_CONSTANTS'] = array();" << endl <<
      endl;
  }
}

/**
 * Prints standard php includes
 */
string t_php_generator::php_includes() {
  return
    string("include_once $GLOBALS['THRIFT_ROOT'].'/Thrift.php';\n\n");
}

/**
 * Close up (or down) some filez.
 */
void t_php_generator::close_generator() {
  // Close types file
  f_types_ << "?>" << endl;
  f_types_.close();

  if (!program_->get_consts().empty()) {
    f_consts_ << "?>" << endl;
    f_consts_.close();
  }
}

/**
 * Generates a typedef. This is not done in PHP, types are all implicit.
 *
 * @param ttypedef The type definition
 */
void t_php_generator::generate_typedef(t_typedef* ttypedef) {
  (void) ttypedef;
}

/**
 * Generates code for an enumerated type. Since define is expensive to lookup
 * in PHP, we use a global array for this.
 *
 * @param tenum The enumeration
 */
void t_php_generator::generate_enum(t_enum* tenum) {
  f_types_ <<
    "$GLOBALS['" << php_namespace(tenum->get_program()) << "E_" << tenum->get_name() << "'] = array(" << endl;

  vector<t_enum_value*> constants = tenum->get_constants();
  vector<t_enum_value*>::iterator c_iter;
  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    int value = (*c_iter)->get_value();
    f_types_ <<
      "  '" << (*c_iter)->get_name() << "' => " << value << "," << endl;
  }

  f_types_ <<
    ");" << endl << endl;


  // We're also doing it this way to see how it performs. It's more legible
  // code but you can't do things like an 'extract' on it, which is a bit of
  // a downer.
  f_types_ <<
    "final class " << tenum->get_name() << " {" << endl;
  indent_up();

  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    int value = (*c_iter)->get_value();
    indent(f_types_) <<
      "const " << (*c_iter)->get_name() << " = " << value << ";" << endl;
  }

  indent(f_types_) <<
    "static public $__names = array(" << endl;
  for (c_iter = constants.begin(); c_iter != constants.end(); ++c_iter) {
    int value = (*c_iter)->get_value();
    indent(f_types_) <<
      "  " << value << " => '" << (*c_iter)->get_name() << "'," << endl;
  }
  indent(f_types_) <<
    ");" << endl;

  indent_down();
  f_types_ << "}" << endl << endl;
}

/**
 * Generate a constant value
 */
void t_php_generator::generate_const(t_const* tconst) {
  t_type* type = tconst->get_type();
  string name = tconst->get_name();
  t_const_value* value = tconst->get_value();

  f_consts_ << "$GLOBALS['" << php_namespace_constant(get_program()) << program_name_ << "_CONSTANTS']['" << name << "'] = ";
  f_consts_ << render_const_value(type, value);
  f_consts_ << ";" << endl << endl;
}

/**
 * Prints the value of a constant with the given type. Note that type checking
 * is NOT performed in this function as it is always run beforehand using the
 * validate_types method in main.cc
 */
string t_php_generator::render_const_value(t_type* type, t_const_value* value) {
  std::ostringstream out;
  type = get_true_type(type);
  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
    case t_base_type::TYPE_STRING:
      out << '"' << get_escaped_string(value) << '"';
      break;
    case t_base_type::TYPE_BOOL:
      out << (value->get_integer() > 0 ? "true" : "false");
      break;
    case t_base_type::TYPE_BYTE:
    case t_base_type::TYPE_I16:
    case t_base_type::TYPE_I32:
    case t_base_type::TYPE_I64:
      out << value->get_integer();
      break;
    case t_base_type::TYPE_DOUBLE:
      if (value->get_type() == t_const_value::CV_INTEGER) {
        out << value->get_integer();
      } else {
        out << value->get_double();
      }
      break;
    default:
      throw "compiler error: no const of base type " + t_base_type::t_base_name(tbase);
    }
  } else if (type->is_enum()) {
    indent(out) << value->get_integer();
  } else if (type->is_struct() || type->is_xception()) {
    out << "new " << NS_ROOT << php_namespace(type->get_program()) << type->get_name() << "(array(" << endl;
    indent_up();
    const vector<t_field*>& fields = ((t_struct*)type)->get_members();
    vector<t_field*>::const_iterator f_iter;
    const map<t_const_value*, t_const_value*>& val = value->get_map();
    map<t_const_value*, t_const_value*>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      t_type* field_type = NULL;
      for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        if ((*f_iter)->get_name() == v_iter->first->get_string()) {
          field_type = (*f_iter)->get_type();
        }
      }
      if (field_type == NULL) {
        throw "type error: " + type->get_name() + " has no field " + v_iter->first->get_string();
      }
      out << indent();
      out << render_const_value(g_type_string, v_iter->first);
      out << " => ";
      out << render_const_value(field_type, v_iter->second);
      out << "," << endl;
    }
    indent_down();
    indent(out) << "))";
  } else if (type->is_map()) {
    t_type* ktype = ((t_map*)type)->get_key_type();
    t_type* vtype = ((t_map*)type)->get_val_type();
    out << "array(" << endl;
    indent_up();
    const map<t_const_value*, t_const_value*>& val = value->get_map();
    map<t_const_value*, t_const_value*>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      out << indent();
      out << render_const_value(ktype, v_iter->first);
      out << " => ";
      out << render_const_value(vtype, v_iter->second);
      out << "," << endl;
    }
    indent_down();
    indent(out) << ")";
  } else if (type->is_list() || type->is_set()) {
    t_type* etype;
    if (type->is_list()) {
      etype = ((t_list*)type)->get_elem_type();
    } else {
      etype = ((t_set*)type)->get_elem_type();
    }
    out << "array(" << endl;
    indent_up();
    const vector<t_const_value*>& val = value->get_list();
    vector<t_const_value*>::const_iterator v_iter;
    for (v_iter = val.begin(); v_iter != val.end(); ++v_iter) {
      out << indent();
      out << render_const_value(etype, *v_iter);
      if (type->is_set()) {
        out << " => true";
      }
      out << "," << endl;
    }
    indent_down();
    indent(out) << ")";
  }
  return out.str();
}

/**
 * Make a struct
 */
void t_php_generator::generate_struct(t_struct* tstruct) {
  generate_php_struct(tstruct, false);
}

/**
 * Generates a struct definition for a thrift exception. Basically the same
 * as a struct but extends the Exception class.
 *
 * @param txception The struct definition
 */
void t_php_generator::generate_xception(t_struct* txception) {
  generate_php_struct(txception, true);
}

/**
 * Structs can be normal or exceptions.
 */
void t_php_generator::generate_php_struct(t_struct* tstruct,
                                          bool is_exception) {
  generate_php_struct_definition(f_types_, tstruct, is_exception);
}

void t_php_generator::generate_php_type_spec(ofstream& out,
                                             t_type* t) {
  t = get_true_type(t);
  indent(out) << "'type' => " << NS_ROOT << type_to_enum(t) << "," << endl;

  if (t->is_base_type() || t->is_enum()) {
    // Noop, type is all we need
  } else if (t->is_struct() || t->is_xception()) {
    indent(out) << "'class' => '" << php_namespace(t->get_program()) << t->get_name() <<"'," << endl;
  } else if (t->is_map()) {
    t_type* ktype = get_true_type(((t_map*)t)->get_key_type());
    t_type* vtype = get_true_type(((t_map*)t)->get_val_type());
    indent(out) << "'ktype' => " << NS_ROOT << type_to_enum(ktype) << "," << endl;
    indent(out) << "'vtype' => " << NS_ROOT << type_to_enum(vtype) << "," << endl;
    indent(out) << "'key' => array(" << endl;
    indent_up();
    generate_php_type_spec(out, ktype);
    indent_down();
    indent(out) << ")," << endl;
    indent(out) << "'val' => array(" << endl;
    indent_up();
    generate_php_type_spec(out, vtype);
    indent(out) << ")," << endl;
    indent_down();
  } else if (t->is_list() || t->is_set()) {
    t_type* etype;
    if (t->is_list()) {
      etype = get_true_type(((t_list*)t)->get_elem_type());
    } else {
      etype = get_true_type(((t_set*)t)->get_elem_type());
    }
    indent(out) << "'etype' => " << NS_ROOT << type_to_enum(etype) <<"," << endl;
    indent(out) << "'elem' => array(" << endl;
    indent_up();
    generate_php_type_spec(out, etype);
    indent(out) << ")," << endl;
    indent_down();
  } else {
    throw "compiler error: no type for php struct spec field";
  }

}

/**
 * Generates the struct specification structure, which fully qualifies enough
 * type information to generalize serialization routines.
 */
void t_php_generator::generate_php_struct_spec(ofstream& out,
                                               t_struct* tstruct) {
  indent(out) << "if (!isset(self::$_TSPEC)) {" << endl;
  indent_up();

  indent(out) << "self::$_TSPEC = array(" << endl;
  indent_up();

  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;
  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    t_type* t = get_true_type((*m_iter)->get_type());
    indent(out) << (*m_iter)->get_key() << " => array(" << endl;
    indent_up();
    out <<
      indent() << "'var' => '" << (*m_iter)->get_name() << "'," << endl;
    generate_php_type_spec(out, t);
    indent(out) << ")," << endl;
    indent_down();
  }

  indent_down();
  indent(out) << "  );" << endl;
  indent_down();
  indent(out) << "}" << endl;
}


void t_php_generator::generate_php_struct_definition(ofstream& out,
                                                     t_struct* tstruct,
                                                     bool is_exception) {
  if (autoload_) {
    // Make output file
    ofstream autoload_out;
    string f_struct = program_name_+"."+(tstruct->get_name())+".php";
    string f_struct_name = package_dir_+f_struct;
    autoload_out.open(f_struct_name.c_str());
    autoload_out << "<?php" << endl
      << "/**" << endl << " *  @generated" << endl << " */" << endl;
    if(namespace_php_) autoload_out << "namespace " << php_namespace_suffix(tstruct->get_program()) << ";" << endl;
    _generate_php_struct_definition(autoload_out, tstruct, is_exception);
    autoload_out << endl << "?>" << endl;
    autoload_out.close();

    f_types_ <<
      "$GLOBALS['THRIFT_AUTOLOAD']['" <<
      lowercase(php_namespace_autoload(tstruct->get_program()) + tstruct->get_name()) <<
      "'] = '" << program_name_ << "/" << f_struct << "';" << endl;

  } else {
    _generate_php_struct_definition(out, tstruct, is_exception);
  }
}

/**
 * Generates a struct definition for a thrift data type. This is nothing in PHP
 * where the objects are all just associative arrays (unless of course we
 * decide to start using objects for them...)
 *
 * @param tstruct The struct definition
 */
void t_php_generator::_generate_php_struct_definition(ofstream& out,
                                                     t_struct* tstruct,
                                                     bool is_exception) {
  const vector<t_field*>& members = tstruct->get_members();
  vector<t_field*>::const_iterator m_iter;

  out <<
    "class " << php_namespace_declaration(tstruct);
  if (is_exception) {
    out << " extends " << NS_ROOT << "TException";
  } else if (oop_) {
    out << " extends " << NS_ROOT << "TBase";
  }
  out <<
    " {" << endl;
  indent_up();

  indent(out) << "static $_TSPEC;" << endl << endl;

  for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
    string dval = "null";
    t_type* t = get_true_type((*m_iter)->get_type());
    if ((*m_iter)->get_value() != NULL && !(t->is_struct() || t->is_xception())) {
      dval = render_const_value((*m_iter)->get_type(), (*m_iter)->get_value());
    }
    indent(out) <<
      "public $" << (*m_iter)->get_name() << " = " << dval << ";" << endl;
  }

  out << endl;

  // Generate constructor from array
  string param = (members.size() > 0) ? "$vals=null" : "";
  out <<
    indent() << "public function __construct(" << param << ") {" << endl;
  indent_up();

  generate_php_struct_spec(out, tstruct);

  if (members.size() > 0) {
    for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
      t_type* t = get_true_type((*m_iter)->get_type());
      if ((*m_iter)->get_value() != NULL && (t->is_struct() || t->is_xception())) {
        indent(out) << "$this->" << (*m_iter)->get_name() << " = " << render_const_value(t, (*m_iter)->get_value()) << ";" << endl;
      }
    }
    out <<
      indent() << "if (is_array($vals)) {" << endl;
    indent_up();
    if (oop_) {
      out << indent() << "parent::__construct(self::$_TSPEC, $vals);" << endl;
    } else {
      for (m_iter = members.begin(); m_iter != members.end(); ++m_iter) {
        out <<
          indent() << "if (isset($vals['" << (*m_iter)->get_name() << "'])) {" << endl <<
          indent() << "  $this->" << (*m_iter)->get_name() << " = $vals['" << (*m_iter)->get_name() << "'];" << endl <<
          indent() << "}" << endl;
      }
    }
    indent_down();
    out <<
      indent() << "}" << endl;
  }
  scope_down(out);
  out << endl;

  out <<
    indent() << "public function getName() {" << endl <<
    indent() << "  return '" << tstruct->get_name() << "';" << endl <<
    indent() << "}" << endl <<
    endl;

  generate_php_struct_reader(out, tstruct);
  generate_php_struct_writer(out, tstruct);

  indent_down();
  out <<
    indent() << "}" << endl <<
    endl;
}

/**
 * Generates the read() method for a struct
 */
void t_php_generator::generate_php_struct_reader(ofstream& out,
                                                 t_struct* tstruct) {
  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;

  indent(out) <<
    "public function read($input)" << endl;
  scope_up(out);

  if (oop_) {
    indent(out) << "return $this->_read('" << tstruct->get_name() << "', self::$_TSPEC, $input);" << endl;
    scope_down(out);
    return;
  }

  out <<
    indent() << "$xfer = 0;" << endl <<
    indent() << "$fname = null;" << endl <<
    indent() << "$ftype = 0;" << endl <<
    indent() << "$fid = 0;" << endl;

  // Declare stack tmp variables
  if (!binary_inline_) {
    indent(out) <<
      "$xfer += $input->readStructBegin($fname);" << endl;
  }

  // Loop over reading in fields
  indent(out) <<
    "while (true)" << endl;

    scope_up(out);

    // Read beginning field marker
    if (binary_inline_) {
      t_field fftype(g_type_byte, "ftype");
      t_field ffid(g_type_i16, "fid");
      generate_deserialize_field(out, &fftype);
      out <<
        indent() << "if ($ftype == " << NS_ROOT << "TType::STOP) {" << endl <<
        indent() << "  break;" << endl <<
        indent() << "}" << endl;
      generate_deserialize_field(out, &ffid);
    } else {
      indent(out) <<
        "$xfer += $input->readFieldBegin($fname, $ftype, $fid);" << endl;
      // Check for field STOP marker and break
      indent(out) <<
        "if ($ftype == " << NS_ROOT << "TType::STOP) {" << endl;
      indent_up();
      indent(out) <<
        "break;" << endl;
      indent_down();
      indent(out) <<
        "}" << endl;
    }

    // Switch statement on the field we are reading
    indent(out) <<
      "switch ($fid)" << endl;

      scope_up(out);

      // Generate deserialization code for known cases
      for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
        indent(out) <<
          "case " << (*f_iter)->get_key() << ":" << endl;
        indent_up();
        indent(out) << "if ($ftype == " << NS_ROOT << type_to_enum((*f_iter)->get_type()) << ") {" << endl;
        indent_up();
        generate_deserialize_field(out, *f_iter, "this->");
        indent_down();
        out <<
          indent() << "} else {" << endl;
        if (binary_inline_) {
          indent(out) <<  "  $xfer += " << NS_ROOT << "TProtocol::skipBinary($input, $ftype);" << endl;
        } else {
          indent(out) <<  "  $xfer += $input->skip($ftype);" << endl;
        }
        out <<
          indent() << "}" << endl <<
          indent() << "break;" << endl;
        indent_down();
      }

      // In the default case we skip the field
      indent(out) <<  "default:" << endl;
      if (binary_inline_) {
        indent(out) <<  "  $xfer += " << NS_ROOT << "TProtocol::skipBinary($input, $ftype);" << endl;
      } else {
        indent(out) <<  "  $xfer += $input->skip($ftype);" << endl;
      }
      indent(out) <<  "  break;" << endl;

      scope_down(out);

    if (!binary_inline_) {
      // Read field end marker
      indent(out) <<
        "$xfer += $input->readFieldEnd();" << endl;
    }

    scope_down(out);

  if (!binary_inline_) {
    indent(out) <<
      "$xfer += $input->readStructEnd();" << endl;
  }

  indent(out) <<
    "return $xfer;" << endl;

  indent_down();
  out <<
    indent() << "}" << endl <<
    endl;
}

/**
 * Generates the write() method for a struct
 */
void t_php_generator::generate_php_struct_writer(ofstream& out,
                                                 t_struct* tstruct) {
  string name = tstruct->get_name();
  const vector<t_field*>& fields = tstruct->get_sorted_members();
  vector<t_field*>::const_iterator f_iter;

  if (binary_inline_) {
    indent(out) <<
      "public function write(&$output) {" << endl;
  } else {
    indent(out) <<
      "public function write($output) {" << endl;
  }
  indent_up();

  if (oop_) {
    indent(out) << "return $this->_write('" << tstruct->get_name() << "', self::$_TSPEC, $output);" << endl;
    scope_down(out);
    return;
  }

  indent(out) <<
    "$xfer = 0;" << endl;

  if (!binary_inline_) {
    indent(out) <<
      "$xfer += $output->writeStructBegin('" << name << "');" << endl;
  }

  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    out <<
      indent() << "if ($this->" << (*f_iter)->get_name() << " !== null) {" << endl;
    indent_up();

    t_type* type = get_true_type((*f_iter)->get_type());
    string expect;
    if (type->is_container()) {
      expect = "array";
    } else if (type->is_struct()) {
      expect = "object";
    }
    if (!expect.empty()) {
      out <<
        indent() << "if (!is_" << expect << "($this->" << (*f_iter)->get_name() << ")) {" << endl;
      indent_up();
      out <<
        indent() << "throw new " << NS_ROOT << "TProtocolException('Bad type in structure.', " << NS_ROOT << "TProtocolException::INVALID_DATA);" << endl;
      scope_down(out);
    }

    // Write field header
    if (binary_inline_) {
      out <<
        indent() << "$output .= pack('c', " << type_to_enum((*f_iter)->get_type()) << ");" << endl <<
        indent() << "$output .= pack('n', " << (*f_iter)->get_key() << ");" << endl;
    } else {
      indent(out) <<
        "$xfer += $output->writeFieldBegin(" <<
        "'" << (*f_iter)->get_name() << "', " <<
        NS_ROOT << type_to_enum((*f_iter)->get_type()) << ", " <<
        (*f_iter)->get_key() << ");" << endl;
    }

    // Write field contents
    generate_serialize_field(out, *f_iter, "this->");

    // Write field closer
    if (!binary_inline_) {
      indent(out) <<
        "$xfer += $output->writeFieldEnd();" << endl;
    }

    indent_down();
    indent(out) <<
      "}" << endl;
  }

  if (binary_inline_) {
    out <<
      indent() << "$output .= pack('c', " << NS_ROOT << "TType::STOP);" << endl;
  } else {
    out <<
      indent() << "$xfer += $output->writeFieldStop();" << endl <<
      indent() << "$xfer += $output->writeStructEnd();" << endl;
  }

  out <<
    indent() << "return $xfer;" << endl;

  indent_down();
  out <<
    indent() << "}" << endl <<
    endl;
}

/**
 * Generates a thrift service.
 *
 * @param tservice The service definition
 */
void t_php_generator::generate_service(t_service* tservice) {
  string f_service_name = package_dir_+service_name_+".php";
  f_service_.open(f_service_name.c_str());

  f_service_ << "<?php" << endl;
  if(namespace_php_) f_service_ << "namespace " << php_namespace_suffix(tservice->get_program()) << ";" << endl;
  f_service_ << autogen_comment() <<
    php_includes();

  f_service_ <<
    "include_once $GLOBALS['THRIFT_ROOT'].'/packages/" << php_path(program_) << "/" << program_name_ << "_types.php';" << endl;

  if (tservice->get_extends() != NULL) {
    f_service_ <<
      "include_once $GLOBALS['THRIFT_ROOT'].'/packages/" << php_path(tservice->get_extends()->get_program()) << "/" << tservice->get_extends()->get_name() << ".php';" << endl;
  }

  f_service_ <<
    endl;

  // Generate the three main parts of the service (well, two for now in PHP)
  generate_service_interface(tservice);
  if (rest_) {
    generate_service_rest(tservice);
  }
  generate_service_client(tservice);
  generate_service_helpers(tservice);
  if (phps_) {
    generate_service_processor(tservice);
  }

  // Close service file
  f_service_ << "?>" << endl;
  f_service_.close();
}

/**
 * Generates a service server definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_php_generator::generate_service_processor(t_service* tservice) {
  // Generate the dispatch methods
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;

  string extends = "";
  string extends_processor = "";
  if (tservice->get_extends() != NULL) {
    extends = tservice->get_extends()->get_name();
    extends_processor = " extends " + php_namespace(tservice->get_extends()->get_program()) + extends + "Processor";
  }

  // Generate the header portion
  f_service_ <<
    "class " << service_name_ << "Processor" << extends_processor << " {" << endl;
  indent_up();

  if (extends.empty()) {
    f_service_ <<
      indent() << "protected $handler_ = null;" << endl;
  }

  f_service_ <<
    indent() << "public function __construct($handler) {" << endl;
  if (extends.empty()) {
    f_service_ <<
      indent() << "  $this->handler_ = $handler;" << endl;
  } else {
    f_service_ <<
      indent() << "  parent::__construct($handler);" << endl;
  }
  f_service_ <<
    indent() << "}" << endl <<
    endl;

  // Generate the server implementation
  indent(f_service_) <<
    "public function process($input, $output) {" << endl;
  indent_up();

  f_service_ <<
    indent() << "$rseqid = 0;" << endl <<
    indent() << "$fname = null;" << endl <<
    indent() << "$mtype = 0;" << endl <<
    endl;

  if (binary_inline_) {
    t_field ffname(g_type_string, "fname");
    t_field fmtype(g_type_byte, "mtype");
    t_field fseqid(g_type_i32, "rseqid");
    generate_deserialize_field(f_service_, &ffname, "", true);
    generate_deserialize_field(f_service_, &fmtype, "", true);
    generate_deserialize_field(f_service_, &fseqid, "", true);
  } else {
    f_service_ <<
      indent() << "$input->readMessageBegin($fname, $mtype, $rseqid);" << endl;
  }

  // HOT: check for method implementation
  f_service_ <<
    indent() << "$methodname = 'process_'.$fname;" << endl <<
    indent() << "if (!method_exists($this, $methodname)) {" << endl;
  if (binary_inline_) {
    f_service_ <<
      indent() << "  throw new Exception('Function '.$fname.' not implemented.');" << endl;
  } else {
    f_service_ <<
      indent() << "  $input->skip(" << NS_ROOT << "TType::STRUCT);" << endl <<
      indent() << "  $input->readMessageEnd();" << endl <<
      indent() << "  $x = new " << NS_ROOT << "TApplicationException('Function '.$fname.' not implemented.', " << NS_ROOT << "TApplicationException::UNKNOWN_METHOD);" << endl <<
      indent() << "  $output->writeMessageBegin($fname, " << NS_ROOT << "TMessageType::EXCEPTION, $rseqid);" << endl <<
      indent() << "  $x->write($output);" << endl <<
      indent() << "  $output->writeMessageEnd();" << endl <<
      indent() << "  $output->getTransport()->flush();" << endl <<
      indent() << "  return;" << endl;
  }
  f_service_ <<
    indent() << "}" << endl <<
    indent() << "$this->$methodname($rseqid, $input, $output);" << endl <<
    indent() << "return true;" << endl;
  indent_down();
  f_service_ <<
    indent() << "}" << endl <<
    endl;

  // Generate the process subfunctions
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    generate_process_function(tservice, *f_iter);
  }

  indent_down();
  f_service_ << "}" << endl;
}

/**
 * Generates a process function definition.
 *
 * @param tfunction The function to write a dispatcher for
 */
void t_php_generator::generate_process_function(t_service* tservice,
                                                t_function* tfunction) {
  // Open function
  indent(f_service_) <<
    "protected function process_" << tfunction->get_name() <<
    "($seqid, $input, $output) {" << endl;
  indent_up();

  string argsname = php_namespace(tservice->get_program()) + service_name_ + "_" + tfunction->get_name() + "_args";
  string resultname = php_namespace(tservice->get_program()) + service_name_ + "_" + tfunction->get_name() + "_result";

  f_service_ <<
    indent() << "$args = new " << argsname << "();" << endl <<
    indent() << "$args->read($input);" << endl;
  if (!binary_inline_) {
    f_service_ <<
      indent() << "$input->readMessageEnd();" << endl;
  }

  t_struct* xs = tfunction->get_xceptions();
  const std::vector<t_field*>& xceptions = xs->get_members();
  vector<t_field*>::const_iterator x_iter;

  // Declare result for non oneway function
  if (!tfunction->is_oneway()) {
    f_service_ <<
      indent() << "$result = new " << resultname << "();" << endl;
  }

  // Try block for a function with exceptions
  if (xceptions.size() > 0) {
    f_service_ <<
      indent() << "try {" << endl;
    indent_up();
  }

  // Generate the function call
  t_struct* arg_struct = tfunction->get_arglist();
  const std::vector<t_field*>& fields = arg_struct->get_members();
  vector<t_field*>::const_iterator f_iter;

  f_service_ << indent();
  if (!tfunction->is_oneway() && !tfunction->get_returntype()->is_void()) {
    f_service_ << "$result->success = ";
  }
  f_service_ <<
    "$this->handler_->" << tfunction->get_name() << "(";
  bool first = true;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      f_service_ << ", ";
    }
    f_service_ << "$args->" << (*f_iter)->get_name();
  }
  f_service_ << ");" << endl;

  if (!tfunction->is_oneway() && xceptions.size() > 0) {
    indent_down();
    for (x_iter = xceptions.begin(); x_iter != xceptions.end(); ++x_iter) {
      f_service_ <<
        indent() << "} catch (" << php_namespace((*x_iter)->get_type()->get_program()) << (*x_iter)->get_type()->get_name() << " $" << (*x_iter)->get_name() << ") {" << endl;
      if (!tfunction->is_oneway()) {
        indent_up();
        f_service_ <<
          indent() << "$result->" << (*x_iter)->get_name() << " = $" << (*x_iter)->get_name() << ";" << endl;
        indent_down();
        f_service_ << indent();
      }
    }
    f_service_ << "}" << endl;
  }

  // Shortcut out here for oneway functions
  if (tfunction->is_oneway()) {
    f_service_ <<
      indent() << "return;" << endl;
    indent_down();
    f_service_ <<
      indent() << "}" << endl;
    return;
  }

  f_service_ <<
    indent() << "$bin_accel = ($output instanceof " << NS_ROOT << "TProtocol::$TBINARYPROTOCOLACCELERATED) && function_exists('thrift_protocol_write_binary');" << endl;

  f_service_ <<
    indent() << "if ($bin_accel)" << endl;
  scope_up(f_service_);

  f_service_ <<
    indent() << "thrift_protocol_write_binary($output, '" << tfunction->get_name() << "', " << NS_ROOT << "TMessageType::REPLY, $result, $seqid, $output->isStrictWrite());" << endl;

  scope_down(f_service_);
  f_service_ <<
    indent() << "else" << endl;
  scope_up(f_service_);

  // Serialize the request header
  if (binary_inline_) {
    f_service_ <<
      indent() << "$buff = pack('N', (0x80010000 | " << NS_ROOT << "TMessageType::REPLY)); " << endl <<
      indent() << "$buff .= pack('N', strlen('" << tfunction->get_name() << "'));" << endl <<
      indent() << "$buff .= '" << tfunction->get_name() << "';" << endl <<
      indent() << "$buff .= pack('N', $seqid);" << endl <<
      indent() << "$result->write($buff);" << endl <<
      indent() << "$output->write($buff);" << endl <<
      indent() << "$output->flush();" << endl;
  } else {
    f_service_ <<
      indent() << "$output->writeMessageBegin('" << tfunction->get_name() << "', " << NS_ROOT << "TMessageType::REPLY, $seqid);" << endl <<
      indent() << "$result->write($output);" << endl <<
      indent() << "$output->writeMessageEnd();" << endl <<
      indent() << "$output->getTransport()->flush();" << endl;
  }

  scope_down(f_service_);

  // Close function
  indent_down();
  f_service_ <<
    indent() << "}" << endl;
}

/**
 * Generates helper functions for a service.
 *
 * @param tservice The service to generate a header definition for
 */
void t_php_generator::generate_service_helpers(t_service* tservice) {
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;

  f_service_ <<
    "// HELPER FUNCTIONS AND STRUCTURES" << endl << endl;

  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    t_struct* ts = (*f_iter)->get_arglist();
    string name = ts->get_name();
    ts->set_name(service_name_ + "_" + name);
    generate_php_struct_definition(f_service_, ts, false);
    generate_php_function_helpers(*f_iter);
    ts->set_name(name);
  }
}

/**
 * Generates a struct and helpers for a function.
 *
 * @param tfunction The function
 */
void t_php_generator::generate_php_function_helpers(t_function* tfunction) {
  if (!tfunction->is_oneway()) {
    t_struct result(program_, service_name_ + "_" + tfunction->get_name() + "_result");
    t_field success(tfunction->get_returntype(), "success", 0);
    if (!tfunction->get_returntype()->is_void()) {
      result.append(&success);
    }

    t_struct* xs = tfunction->get_xceptions();
    const vector<t_field*>& fields = xs->get_members();
    vector<t_field*>::const_iterator f_iter;
    for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
      result.append(*f_iter);
    }

    generate_php_struct_definition(f_service_, &result, false);
  }
}

/**
 * Generates a service interface definition.
 *
 * @param tservice The service to generate a header definition for
 */
void t_php_generator::generate_service_interface(t_service* tservice) {
  string extends = "";
  string extends_if = "";
  if (tservice->get_extends() != NULL) {
    extends = " extends " + php_namespace(tservice->get_extends()->get_program()) + tservice->get_extends()->get_name();
    extends_if = " extends " + php_namespace(tservice->get_extends()->get_program()) + tservice->get_extends()->get_name() + "If";
  }
  f_service_ <<
    "interface " << php_namespace_declaration(tservice) << "If" << extends_if << " {" << endl;
  indent_up();
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    indent(f_service_) <<
      "public function " << function_signature(*f_iter) << ";" << endl;
  }
  indent_down();
  f_service_ <<
    "}" << endl << endl;
}

/**
 * Generates a REST interface
 */
void t_php_generator::generate_service_rest(t_service* tservice) {
  string extends = "";
  string extends_if = "";
  if (tservice->get_extends() != NULL) {
    extends = " extends " + php_namespace(tservice->get_extends()->get_program()) + tservice->get_extends()->get_name();
    extends_if = " extends " + php_namespace(tservice->get_extends()->get_program()) + tservice->get_extends()->get_name() + "Rest";
  }
  f_service_ <<
    "class " << service_name_ << "Rest" << extends_if << " {" << endl;
  indent_up();

  if (extends.empty()) {
    f_service_ <<
      indent() << "protected $impl_;" << endl <<
      endl;
  }

  f_service_ <<
    indent() << "public function __construct($impl) {" << endl <<
    indent() << "  $this->impl_ = $impl;" << endl <<
    indent() << "}" << endl <<
    endl;

  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    indent(f_service_) <<
      "public function " << (*f_iter)->get_name() << "($request) {" << endl;
    indent_up();
    const vector<t_field*>& args = (*f_iter)->get_arglist()->get_members();
    vector<t_field*>::const_iterator a_iter;
    for (a_iter = args.begin(); a_iter != args.end(); ++a_iter) {
      t_type* atype = get_true_type((*a_iter)->get_type());
      string cast = type_to_cast(atype);
      string req = "$request['" + (*a_iter)->get_name() + "']";
      if (atype->is_bool()) {
        f_service_ <<
          indent() << "$" << (*a_iter)->get_name() << " = " << cast << "(!empty(" << req << ") && (" << req << " !== 'false'));" << endl;
      } else {
        f_service_ <<
          indent() << "$" << (*a_iter)->get_name() << " = isset(" << req << ") ? " << cast << req << " : null;" << endl;
      }
      if (atype->is_string() &&
          ((t_base_type*)atype)->is_string_list()) {
        f_service_ <<
          indent() << "$" << (*a_iter)->get_name() << " = explode(',', $" << (*a_iter)->get_name() << ");" << endl;
      } else if (atype->is_map() || atype->is_list()) {
        f_service_ <<
          indent() << "$" << (*a_iter)->get_name() << " = json_decode($" << (*a_iter)->get_name() << ", true);" << endl;
      } else if (atype->is_set()) {
        f_service_ <<
          indent() << "$" << (*a_iter)->get_name() << " = array_fill_keys(json_decode($" << (*a_iter)->get_name() << ", true), 1);" << endl;
      } else if (atype->is_struct() || atype->is_xception()) {
        f_service_ <<
          indent() << "if ($" << (*a_iter)->get_name() << " !== null) {" << endl <<
          indent() << "  $" << (*a_iter)->get_name() << " = new " << php_namespace(atype->get_program()) << atype->get_name() << "(json_decode($" << (*a_iter)->get_name() << ", true));" << endl <<
          indent() << "}" << endl;
      }
    }
    f_service_ <<
      indent() << "return $this->impl_->" << (*f_iter)->get_name() << "(" << argument_list((*f_iter)->get_arglist()) << ");" << endl;
    indent_down();
    indent(f_service_) <<
      "}" << endl <<
      endl;
  }
  indent_down();
  f_service_ <<
    "}" << endl << endl;
}

void t_php_generator::generate_service_client(t_service* tservice) {
  if (autoload_) {
    // Make output file
    ofstream autoload_out;
    string f_struct = program_name_+"."+(tservice->get_name())+".client.php";
    string f_struct_name = package_dir_+f_struct;
    autoload_out.open(f_struct_name.c_str());
    autoload_out << "<?php" << endl
      << "/**" << endl << " *  @generated" << endl << " */" << endl;
    if(namespace_php_) autoload_out << endl << "namespace " << php_namespace_suffix(tservice->get_program()) << ";" << endl << endl;
    _generate_service_client(autoload_out, tservice);
    autoload_out << endl << "?>" << endl;
    autoload_out.close();

    f_service_ <<
      "$GLOBALS['THRIFT_AUTOLOAD']['" <<
      lowercase(php_namespace_autoload(get_program()) + service_name_ + "Client") <<
      "'] = '" << program_name_ << "/" << f_struct << "';" << endl;

  } else {
    _generate_service_client(f_service_, tservice);
  }
}

/**
 * Generates a service client definition.
 *
 * @param tservice The service to generate a server for.
 */
void t_php_generator::_generate_service_client(ofstream& out, t_service* tservice) {
  string extends = "";
  string extends_client = "";
  if (tservice->get_extends() != NULL) {
    extends = tservice->get_extends()->get_name();
    extends_client = " extends " + php_namespace(tservice->get_extends()->get_program()) + extends + "Client";
  }

  out <<
    "class " << php_namespace_declaration(tservice) << "Client" << extends_client << " implements " <<  php_namespace(tservice->get_program()) << service_name_ << "If {" << endl;
  indent_up();

  // Private members
  if (extends.empty()) {
    out <<
      indent() << "protected $input_ = null;" << endl <<
      indent() << "protected $output_ = null;" << endl <<
      endl;
    out <<
      indent() << "protected $seqid_ = 0;" << endl <<
      endl;
  }

  // Constructor function
  out <<
    indent() << "public function __construct($input, $output=null) {" << endl;
  if (!extends.empty()) {
    out <<
      indent() << "  parent::__construct($input, $output);" << endl;
  } else {
    out <<
      indent() << "  $this->input_ = $input;" << endl <<
      indent() << "  $this->output_ = $output ? $output : $input;" << endl;
  }
  out <<
    indent() << "}" << endl << endl;

  // Generate client method implementations
  vector<t_function*> functions = tservice->get_functions();
  vector<t_function*>::const_iterator f_iter;
  for (f_iter = functions.begin(); f_iter != functions.end(); ++f_iter) {
    t_struct* arg_struct = (*f_iter)->get_arglist();
    const vector<t_field*>& fields = arg_struct->get_members();
    vector<t_field*>::const_iterator fld_iter;
    string funname = (*f_iter)->get_name();

    // Open function
    indent(out) <<
      "public function " << function_signature(*f_iter) << endl;
    scope_up(out);
      indent(out) <<
        "$this->send_" << funname << "(";

      bool first = true;
      for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
        if (first) {
          first = false;
        } else {
          out << ", ";
        }
        out << "$" << (*fld_iter)->get_name();
      }
      out << ");" << endl;

      if (!(*f_iter)->is_oneway()) {
        out << indent();
        if (!(*f_iter)->get_returntype()->is_void()) {
          out << "return ";
        }
        out <<
          "$this->recv_" << funname << "();" << endl;
      }
    scope_down(out);
    out << endl;

    indent(out) <<
      "public function send_" << function_signature(*f_iter) << endl;
    scope_up(out);

      std::string argsname = php_namespace(tservice->get_program()) + service_name_ + "_" + (*f_iter)->get_name() + "_args";

      out <<
        indent() << "$args = new " << argsname << "();" << endl;

      for (fld_iter = fields.begin(); fld_iter != fields.end(); ++fld_iter) {
        out <<
          indent() << "$args->" << (*fld_iter)->get_name() << " = $" << (*fld_iter)->get_name() << ";" << endl;
      }

      out <<
        indent() << "$bin_accel = ($this->output_ instanceof " << NS_ROOT << "TProtocol::$TBINARYPROTOCOLACCELERATED) && function_exists('thrift_protocol_write_binary');" << endl;

      out <<
        indent() << "if ($bin_accel)" << endl;
      scope_up(out);

      out <<
        indent() << "thrift_protocol_write_binary($this->output_, '" << (*f_iter)->get_name() << "', " << NS_ROOT << "TMessageType::CALL, $args, $this->seqid_, $this->output_->isStrictWrite());" << endl;

      scope_down(out);
      out <<
        indent() << "else" << endl;
      scope_up(out);

      // Serialize the request header
      if (binary_inline_) {
        out <<
          indent() << "$buff = pack('N', (0x80010000 | " << NS_ROOT << "TMessageType::CALL));" << endl <<
          indent() << "$buff .= pack('N', strlen('" << funname << "'));" << endl <<
          indent() << "$buff .= '" << funname << "';" << endl <<
          indent() << "$buff .= pack('N', $this->seqid_);" << endl;
      } else {
        out <<
          indent() << "$this->output_->writeMessageBegin('" << (*f_iter)->get_name() << "', " << NS_ROOT << "TMessageType::CALL, $this->seqid_);" << endl;
      }

      // Write to the stream
      if (binary_inline_) {
        out <<
          indent() << "$args->write($buff);" << endl <<
          indent() << "$this->output_->write($buff);" << endl <<
          indent() << "$this->output_->flush();" << endl;
      } else {
        out <<
          indent() << "$args->write($this->output_);" << endl <<
          indent() << "$this->output_->writeMessageEnd();" << endl <<
          indent() << "$this->output_->getTransport()->flush();" << endl;
      }

    scope_down(out);

    scope_down(out);


    if (!(*f_iter)->is_oneway()) {
      std::string resultname = php_namespace(tservice->get_program()) + service_name_ + "_" + (*f_iter)->get_name() + "_result";
      t_struct noargs(program_);

      t_function recv_function((*f_iter)->get_returntype(),
                               string("recv_") + (*f_iter)->get_name(),
                               &noargs);
      // Open function
      out <<
        endl <<
        indent() << "public function " << function_signature(&recv_function) << endl;
      scope_up(out);

      out <<
        indent() << "$bin_accel = ($this->input_ instanceof " << NS_ROOT << "TProtocol::$TBINARYPROTOCOLACCELERATED)"
                 << " && function_exists('thrift_protocol_read_binary');" << endl;

      out <<
        indent() << "if ($bin_accel) $result = thrift_protocol_read_binary($this->input_, '" << resultname << "', $this->input_->isStrictRead());" << endl;
      out <<
        indent() << "else" << endl;
      scope_up(out);

      out <<
        indent() << "$rseqid = 0;" << endl <<
        indent() << "$fname = null;" << endl <<
        indent() << "$mtype = 0;" << endl <<
        endl;

      if (binary_inline_) {
        t_field ffname(g_type_string, "fname");
        t_field fseqid(g_type_i32, "rseqid");
        out <<
          indent() << "$ver = unpack('N', $this->input_->readAll(4));" << endl <<
          indent() << "$ver = $ver[1];" << endl <<
          indent() << "$mtype = $ver & 0xff;" << endl <<
          indent() << "$ver = $ver & 0xffff0000;" << endl <<
          indent() << "if ($ver != 0x80010000) throw new " << NS_ROOT << "TProtocolException('Bad version identifier: '.$ver, " << NS_ROOT << "TProtocolException::BAD_VERSION);" << endl;
        generate_deserialize_field(out, &ffname, "", true);
        generate_deserialize_field(out, &fseqid, "", true);
      } else {
        out <<
          indent() << "$this->input_->readMessageBegin($fname, $mtype, $rseqid);" << endl <<
          indent() << "if ($mtype == " << NS_ROOT << "TMessageType::EXCEPTION) {" << endl <<
          indent() << "  $x = new " << NS_ROOT << "TApplicationException();" << endl <<
          indent() << "  $x->read($this->input_);" << endl <<
          indent() << "  $this->input_->readMessageEnd();" << endl <<
          indent() << "  throw $x;" << endl <<
          indent() << "}" << endl;
      }

      out <<
        indent() << "$result = new " << resultname << "();" << endl <<
        indent() << "$result->read($this->input_);" << endl;

      if (!binary_inline_) {
        out <<
          indent() << "$this->input_->readMessageEnd();" << endl;
      }

      scope_down(out);

      // Careful, only return result if not a void function
      if (!(*f_iter)->get_returntype()->is_void()) {
        out <<
          indent() << "if ($result->success !== null) {" << endl <<
          indent() << "  return $result->success;" << endl <<
          indent() << "}" << endl;
      }

      t_struct* xs = (*f_iter)->get_xceptions();
      const std::vector<t_field*>& xceptions = xs->get_members();
      vector<t_field*>::const_iterator x_iter;
      for (x_iter = xceptions.begin(); x_iter != xceptions.end(); ++x_iter) {
        out <<
          indent() << "if ($result->" << (*x_iter)->get_name() << " !== null) {" << endl <<
          indent() << "  throw $result->" << (*x_iter)->get_name() << ";" << endl <<
          indent() << "}" << endl;
      }

      // Careful, only return _result if not a void function
      if ((*f_iter)->get_returntype()->is_void()) {
        indent(out) <<
          "return;" << endl;
      } else {
        out <<
          indent() << "throw new " << NS_ROOT << "Exception(\"" << (*f_iter)->get_name() << " failed: unknown result\");" << endl;
      }

    // Close function
    scope_down(out);
    out << endl;

    }
  }

  indent_down();
  out <<
    "}" << endl << endl;
}

/**
 * Deserializes a field of any type.
 */
void t_php_generator::generate_deserialize_field(ofstream &out,
                                                 t_field* tfield,
                                                 string prefix,
                                                 bool inclass) {
  t_type* type = get_true_type(tfield->get_type());

  if (type->is_void()) {
    throw "CANNOT GENERATE DESERIALIZE CODE FOR void TYPE: " +
      prefix + tfield->get_name();
  }

  string name = prefix + tfield->get_name();

  if (type->is_struct() || type->is_xception()) {
    generate_deserialize_struct(out,
                                (t_struct*)type,
                                 name);
  } else {

    if (type->is_container()) {
      generate_deserialize_container(out, type, name);
    } else if (type->is_base_type() || type->is_enum()) {

      if (binary_inline_) {
        std::string itrans = (inclass ? "$this->input_" : "$input");

        if (type->is_base_type()) {
          t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
          switch (tbase) {
          case t_base_type::TYPE_VOID:
            throw "compiler error: cannot serialize void field in a struct: " +
              name;
            break;
          case t_base_type::TYPE_STRING:
            out <<
              indent() << "$len = unpack('N', " << itrans << "->readAll(4));" << endl <<
              indent() << "$len = $len[1];" << endl <<
              indent() << "if ($len > 0x7fffffff) {" << endl <<
              indent() << "  $len = 0 - (($len - 1) ^ 0xffffffff);" << endl <<
              indent() << "}" << endl <<
              indent() << "$" << name << " = " << itrans << "->readAll($len);" << endl;
            break;
          case t_base_type::TYPE_BOOL:
            out <<
              indent() << "$" << name << " = unpack('c', " << itrans << "->readAll(1));" << endl <<
              indent() << "$" << name << " = (bool)$" << name << "[1];" << endl;
            break;
          case t_base_type::TYPE_BYTE:
            out <<
              indent() << "$" << name << " = unpack('c', " << itrans << "->readAll(1));" << endl <<
              indent() << "$" << name << " = $" << name << "[1];" << endl;
            break;
          case t_base_type::TYPE_I16:
            out <<
              indent() << "$val = unpack('n', " << itrans << "->readAll(2));" << endl <<
              indent() << "$val = $val[1];" << endl <<
              indent() << "if ($val > 0x7fff) {" << endl <<
              indent() << "  $val = 0 - (($val - 1) ^ 0xffff);" << endl <<
              indent() << "}" << endl <<
              indent() << "$" << name << " = $val;" << endl;
            break;
          case t_base_type::TYPE_I32:
            out <<
              indent() << "$val = unpack('N', " << itrans << "->readAll(4));" << endl <<
              indent() << "$val = $val[1];" << endl <<
              indent() << "if ($val > 0x7fffffff) {" << endl <<
              indent() << "  $val = 0 - (($val - 1) ^ 0xffffffff);" << endl <<
              indent() << "}" << endl <<
              indent() << "$" << name << " = $val;" << endl;
            break;
          case t_base_type::TYPE_I64:
            out <<
              indent() << "$arr = unpack('N2', " << itrans << "->readAll(8));" << endl <<
              indent() << "if ($arr[1] & 0x80000000) {" << endl <<
              indent() << "  $arr[1] = $arr[1] ^ 0xFFFFFFFF;" << endl <<
              indent() << "  $arr[2] = $arr[2] ^ 0xFFFFFFFF;" << endl <<
              indent() << "  $" << name << " = 0 - $arr[1]*4294967296 - $arr[2] - 1;" << endl <<
              indent() << "} else {" << endl <<
              indent() << "  $" << name << " = $arr[1]*4294967296 + $arr[2];" << endl <<
              indent() << "}" << endl;
            break;
          case t_base_type::TYPE_DOUBLE:
            out <<
              indent() << "$arr = unpack('d', strrev(" << itrans << "->readAll(8)));" << endl <<
              indent() << "$" << name << " = $arr[1];" << endl;
            break;
          default:
            throw "compiler error: no PHP name for base type " + t_base_type::t_base_name(tbase) + tfield->get_name();
          }
        } else if (type->is_enum()) {
            out <<
              indent() << "$val = unpack('N', " << itrans << "->readAll(4));" << endl <<
              indent() << "$val = $val[1];" << endl <<
              indent() << "if ($val > 0x7fffffff) {" << endl <<
              indent() << "  $val = 0 - (($val - 1) ^ 0xffffffff);" << endl <<
              indent() << "}" << endl <<
              indent() << "$" << name << " = $val;" << endl;
        }
      } else {

        indent(out) <<
          "$xfer += $input->";

        if (type->is_base_type()) {
          t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
          switch (tbase) {
          case t_base_type::TYPE_VOID:
            throw "compiler error: cannot serialize void field in a struct: " +
              name;
            break;
          case t_base_type::TYPE_STRING:
            out << "readString($" << name << ");";
            break;
          case t_base_type::TYPE_BOOL:
            out << "readBool($" << name << ");";
            break;
          case t_base_type::TYPE_BYTE:
            out << "readByte($" << name << ");";
            break;
          case t_base_type::TYPE_I16:
            out << "readI16($" << name << ");";
            break;
          case t_base_type::TYPE_I32:
            out << "readI32($" << name << ");";
            break;
          case t_base_type::TYPE_I64:
            out << "readI64($" << name << ");";
            break;
          case t_base_type::TYPE_DOUBLE:
            out << "readDouble($" << name << ");";
            break;
          default:
            throw "compiler error: no PHP name for base type " + t_base_type::t_base_name(tbase);
          }
        } else if (type->is_enum()) {
          out << "readI32($" << name << ");";
        }
        out << endl;
      }
    } else {
      printf("DO NOT KNOW HOW TO DESERIALIZE FIELD '%s' TYPE '%s'\n",
             tfield->get_name().c_str(), type->get_name().c_str());
    }
  }
}

/**
 * Generates an unserializer for a variable. This makes two key assumptions,
 * first that there is a const char* variable named data that points to the
 * buffer for deserialization, and that there is a variable protocol which
 * is a reference to a TProtocol serialization object.
 */
void t_php_generator::generate_deserialize_struct(ofstream &out,
                                                  t_struct* tstruct,
                                                  string prefix) {
  out <<
    indent() << "$" << prefix << " = new " << php_namespace(tstruct->get_program()) << tstruct->get_name() << "();" << endl <<
    indent() << "$xfer += $" << prefix << "->read($input);" << endl;
}

void t_php_generator::generate_deserialize_container(ofstream &out,
                                                     t_type* ttype,
                                                     string prefix) {
  string size = tmp("_size");
  string ktype = tmp("_ktype");
  string vtype = tmp("_vtype");
  string etype = tmp("_etype");

  t_field fsize(g_type_i32, size);
  t_field fktype(g_type_byte, ktype);
  t_field fvtype(g_type_byte, vtype);
  t_field fetype(g_type_byte, etype);

  out <<
    indent() << "$" << prefix << " = array();" << endl <<
    indent() << "$" << size << " = 0;" << endl;

  // Declare variables, read header
  if (ttype->is_map()) {
    out <<
      indent() << "$" << ktype << " = 0;" << endl <<
      indent() << "$" << vtype << " = 0;" << endl;
    if (binary_inline_) {
      generate_deserialize_field(out, &fktype);
      generate_deserialize_field(out, &fvtype);
      generate_deserialize_field(out, &fsize);
    } else {
      out <<
        indent() << "$xfer += $input->readMapBegin(" <<
        "$" << ktype << ", $" << vtype << ", $" << size << ");" << endl;
    }
  } else if (ttype->is_set()) {
    if (binary_inline_) {
      generate_deserialize_field(out, &fetype);
      generate_deserialize_field(out, &fsize);
    } else {
      out <<
        indent() << "$" << etype << " = 0;" << endl <<
        indent() << "$xfer += $input->readSetBegin(" <<
        "$" << etype << ", $" << size << ");" << endl;
    }
  } else if (ttype->is_list()) {
    if (binary_inline_) {
      generate_deserialize_field(out, &fetype);
      generate_deserialize_field(out, &fsize);
    } else {
      out <<
        indent() << "$" << etype << " = 0;" << endl <<
        indent() << "$xfer += $input->readListBegin(" <<
        "$" << etype << ", $" << size << ");" << endl;
    }
  }

  // For loop iterates over elements
  string i = tmp("_i");
  indent(out) <<
    "for ($" <<
    i << " = 0; $" << i << " < $" << size << "; ++$" << i << ")" << endl;

    scope_up(out);

    if (ttype->is_map()) {
      generate_deserialize_map_element(out, (t_map*)ttype, prefix);
    } else if (ttype->is_set()) {
      generate_deserialize_set_element(out, (t_set*)ttype, prefix);
    } else if (ttype->is_list()) {
      generate_deserialize_list_element(out, (t_list*)ttype, prefix);
    }

    scope_down(out);

  if (!binary_inline_) {
    // Read container end
    if (ttype->is_map()) {
      indent(out) << "$xfer += $input->readMapEnd();" << endl;
    } else if (ttype->is_set()) {
      indent(out) << "$xfer += $input->readSetEnd();" << endl;
    } else if (ttype->is_list()) {
      indent(out) << "$xfer += $input->readListEnd();" << endl;
    }
  }
}


/**
 * Generates code to deserialize a map
 */
void t_php_generator::generate_deserialize_map_element(ofstream &out,
                                                       t_map* tmap,
                                                       string prefix) {
  string key = tmp("key");
  string val = tmp("val");
  t_field fkey(tmap->get_key_type(), key);
  t_field fval(tmap->get_val_type(), val);

  indent(out) <<
    declare_field(&fkey, true, true) << endl;
  indent(out) <<
    declare_field(&fval, true, true) << endl;

  generate_deserialize_field(out, &fkey);
  generate_deserialize_field(out, &fval);

  indent(out) <<
    "$" << prefix << "[$" << key << "] = $" << val << ";" << endl;
}

void t_php_generator::generate_deserialize_set_element(ofstream &out,
                                                       t_set* tset,
                                                       string prefix) {
  string elem = tmp("elem");
  t_field felem(tset->get_elem_type(), elem);

  indent(out) <<
    "$" << elem << " = null;" << endl;

  generate_deserialize_field(out, &felem);

  indent(out) << "if (is_scalar($" << elem << ")) {" << endl;
  indent(out) << "  $" << prefix << "[$" << elem << "] = true;" << endl;
  indent(out) << "} else {" << endl;
  indent(out) << "  $" << prefix << " []= $" << elem << ";" << endl;
  indent(out) << "}" << endl;
}

void t_php_generator::generate_deserialize_list_element(ofstream &out,
                                                        t_list* tlist,
                                                        string prefix) {
  string elem = tmp("elem");
  t_field felem(tlist->get_elem_type(), elem);

  indent(out) <<
    "$" << elem << " = null;" << endl;

  generate_deserialize_field(out, &felem);

  indent(out) <<
    "$" << prefix << " []= $" << elem << ";" << endl;
}


/**
 * Serializes a field of any type.
 *
 * @param tfield The field to serialize
 * @param prefix Name to prepend to field name
 */
void t_php_generator::generate_serialize_field(ofstream &out,
                                               t_field* tfield,
                                               string prefix) {
  t_type* type = get_true_type(tfield->get_type());

  // Do nothing for void types
  if (type->is_void()) {
    throw "CANNOT GENERATE SERIALIZE CODE FOR void TYPE: " +
      prefix + tfield->get_name();
  }

  if (type->is_struct() || type->is_xception()) {
    generate_serialize_struct(out,
                              (t_struct*)type,
                              prefix + tfield->get_name());
  } else if (type->is_container()) {
    generate_serialize_container(out,
                                 type,
                                 prefix + tfield->get_name());
  } else if (type->is_base_type() || type->is_enum()) {

    string name = prefix + tfield->get_name();

    if (binary_inline_) {
      if (type->is_base_type()) {
        t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
        switch (tbase) {
        case t_base_type::TYPE_VOID:
          throw
            "compiler error: cannot serialize void field in a struct: " + name;
          break;
        case t_base_type::TYPE_STRING:
          out <<
            indent() << "$output .= pack('N', strlen($" << name << "));" << endl <<
            indent() << "$output .= $" << name << ";" << endl;
          break;
        case t_base_type::TYPE_BOOL:
          out <<
            indent() << "$output .= pack('c', $" << name << " ? 1 : 0);" << endl;
          break;
        case t_base_type::TYPE_BYTE:
          out <<
            indent() << "$output .= pack('c', $" << name << ");" << endl;
          break;
        case t_base_type::TYPE_I16:
          out <<
            indent() << "$output .= pack('n', $" << name << ");" << endl;
          break;
        case t_base_type::TYPE_I32:
          out <<
            indent() << "$output .= pack('N', $" << name << ");" << endl;
          break;
        case t_base_type::TYPE_I64:
          out <<
            indent() << "$output .= pack('N2', $" << name << " >> 32, $" << name << " & 0xFFFFFFFF);" << endl;
          break;
        case t_base_type::TYPE_DOUBLE:
          out <<
            indent() << "$output .= strrev(pack('d', $" << name << "));" << endl;
          break;
        default:
          throw "compiler error: no PHP name for base type " + t_base_type::t_base_name(tbase);
        }
      } else if (type->is_enum()) {
        out <<
          indent() << "$output .= pack('N', $" << name << ");" << endl;
      }
    } else {

      indent(out) <<
        "$xfer += $output->";

      if (type->is_base_type()) {
        t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
        switch (tbase) {
        case t_base_type::TYPE_VOID:
          throw
            "compiler error: cannot serialize void field in a struct: " + name;
          break;
        case t_base_type::TYPE_STRING:
          out << "writeString($" << name << ");";
          break;
        case t_base_type::TYPE_BOOL:
          out << "writeBool($" << name << ");";
          break;
        case t_base_type::TYPE_BYTE:
          out << "writeByte($" << name << ");";
          break;
        case t_base_type::TYPE_I16:
          out << "writeI16($" << name << ");";
          break;
        case t_base_type::TYPE_I32:
          out << "writeI32($" << name << ");";
          break;
        case t_base_type::TYPE_I64:
          out << "writeI64($" << name << ");";
          break;
        case t_base_type::TYPE_DOUBLE:
          out << "writeDouble($" << name << ");";
          break;
        default:
          throw "compiler error: no PHP name for base type " + t_base_type::t_base_name(tbase);
        }
      } else if (type->is_enum()) {
        out << "writeI32($" << name << ");";
      }
      out << endl;
    }
  } else {
    printf("DO NOT KNOW HOW TO SERIALIZE FIELD '%s%s' TYPE '%s'\n",
           prefix.c_str(),
           tfield->get_name().c_str(),
           type->get_name().c_str());
  }
}

/**
 * Serializes all the members of a struct.
 *
 * @param tstruct The struct to serialize
 * @param prefix  String prefix to attach to all fields
 */
void t_php_generator::generate_serialize_struct(ofstream &out,
                                                t_struct* tstruct,
                                                string prefix) {
  (void) tstruct;
  indent(out) <<
    "$xfer += $" << prefix << "->write($output);" << endl;
}

/**
 * Writes out a container
 */
void t_php_generator::generate_serialize_container(ofstream &out,
                                                   t_type* ttype,
                                                   string prefix) {
  scope_up(out);

  if (ttype->is_map()) {
    if (binary_inline_) {
      out <<
        indent() << "$output .= pack('c', " << type_to_enum(((t_map*)ttype)->get_key_type()) << ");" << endl <<
        indent() << "$output .= pack('c', " << type_to_enum(((t_map*)ttype)->get_val_type()) << ");" << endl <<
        indent() << "$output .= strrev(pack('l', count($" << prefix << ")));" << endl;
    } else {
      indent(out) <<
        "$output->writeMapBegin(" <<
        NS_ROOT << type_to_enum(((t_map*)ttype)->get_key_type()) << ", " <<
        NS_ROOT << type_to_enum(((t_map*)ttype)->get_val_type()) << ", " <<
        "count($" << prefix << "));" << endl;
    }
  } else if (ttype->is_set()) {
    if (binary_inline_) {
      out <<
        indent() << "$output .= pack('c', " << type_to_enum(((t_set*)ttype)->get_elem_type()) << ");" << endl <<
        indent() << "$output .= strrev(pack('l', count($" << prefix << ")));" << endl;

    } else {
      indent(out) <<
        "$output->writeSetBegin(" <<
        NS_ROOT << type_to_enum(((t_set*)ttype)->get_elem_type()) << ", " <<
        "count($" << prefix << "));" << endl;
    }
  } else if (ttype->is_list()) {
    if (binary_inline_) {
      out <<
        indent() << "$output .= pack('c', " << type_to_enum(((t_list*)ttype)->get_elem_type()) << ");" << endl <<
        indent() << "$output .= strrev(pack('l', count($" << prefix << ")));" << endl;

    } else {
      indent(out) <<
        "$output->writeListBegin(" <<
        NS_ROOT << type_to_enum(((t_list*)ttype)->get_elem_type()) << ", " <<
        "count($" << prefix << "));" << endl;
    }
  }

  scope_up(out);

  if (ttype->is_map()) {
    string kiter = tmp("kiter");
    string viter = tmp("viter");
    indent(out) <<
      "foreach ($" << prefix << " as " <<
      "$" << kiter << " => $" << viter << ")" << endl;
    scope_up(out);
    generate_serialize_map_element(out, (t_map*)ttype, kiter, viter);
    scope_down(out);
  } else if (ttype->is_set()) {
    string iter = tmp("iter");
    string iter_val = tmp("iter");
    indent(out) <<
      "foreach ($" << prefix << " as $" << iter << " => $" << iter_val << ")" << endl;
    scope_up(out);
    indent(out) << "if (is_scalar($" << iter_val << ")) {" << endl;
    generate_serialize_set_element(out, (t_set*)ttype, iter);
    indent(out) << "} else {" << endl;
    generate_serialize_set_element(out, (t_set*)ttype, iter_val);
    indent(out) << "}" << endl;
    scope_down(out);
  } else if (ttype->is_list()) {
    string iter = tmp("iter");
    indent(out) <<
      "foreach ($" << prefix << " as $" << iter << ")" << endl;
    scope_up(out);
    generate_serialize_list_element(out, (t_list*)ttype, iter);
    scope_down(out);
  }

  scope_down(out);

  if (!binary_inline_) {
    if (ttype->is_map()) {
      indent(out) <<
        "$output->writeMapEnd();" << endl;
    } else if (ttype->is_set()) {
      indent(out) <<
        "$output->writeSetEnd();" << endl;
    } else if (ttype->is_list()) {
      indent(out) <<
        "$output->writeListEnd();" << endl;
    }
  }

  scope_down(out);
}

/**
 * Serializes the members of a map.
 *
 */
void t_php_generator::generate_serialize_map_element(ofstream &out,
                                                     t_map* tmap,
                                                     string kiter,
                                                     string viter) {
  t_field kfield(tmap->get_key_type(), kiter);
  generate_serialize_field(out, &kfield, "");

  t_field vfield(tmap->get_val_type(), viter);
  generate_serialize_field(out, &vfield, "");
}

/**
 * Serializes the members of a set.
 */
void t_php_generator::generate_serialize_set_element(ofstream &out,
                                                     t_set* tset,
                                                     string iter) {
  t_field efield(tset->get_elem_type(), iter);
  generate_serialize_field(out, &efield, "");
}

/**
 * Serializes the members of a list.
 */
void t_php_generator::generate_serialize_list_element(ofstream &out,
                                                      t_list* tlist,
                                                      string iter) {
  t_field efield(tlist->get_elem_type(), iter);
  generate_serialize_field(out, &efield, "");
}

/**
 * Declares a field, which may include initialization as necessary.
 *
 * @param ttype The type
 */
string t_php_generator::declare_field(t_field* tfield, bool init, bool obj) {
  string result = "$" + tfield->get_name();
  if (init) {
    t_type* type = get_true_type(tfield->get_type());
    if (type->is_base_type()) {
      t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
      switch (tbase) {
      case t_base_type::TYPE_VOID:
        break;
      case t_base_type::TYPE_STRING:
        result += " = ''";
        break;
      case t_base_type::TYPE_BOOL:
        result += " = false";
        break;
      case t_base_type::TYPE_BYTE:
      case t_base_type::TYPE_I16:
      case t_base_type::TYPE_I32:
      case t_base_type::TYPE_I64:
        result += " = 0";
        break;
      case t_base_type::TYPE_DOUBLE:
        result += " = 0.0";
        break;
      default:
        throw "compiler error: no PHP initializer for base type " + t_base_type::t_base_name(tbase);
      }
    } else if (type->is_enum()) {
      result += " = 0";
    } else if (type->is_container()) {
      result += " = array()";
    } else if (type->is_struct() || type->is_xception()) {
      if (obj) {
        result += " = new " + php_namespace(type->get_program()) + type->get_name() + "()";
      } else {
        result += " = null";
      }
    }
  }
  return result + ";";
}

/**
 * Renders a function signature of the form 'type name(args)'
 *
 * @param tfunction Function definition
 * @return String of rendered function definition
 */
string t_php_generator::function_signature(t_function* tfunction,
                                           string prefix) {
  return
    prefix + tfunction->get_name() +
    "(" + argument_list(tfunction->get_arglist()) + ")";
}

/**
 * Renders a field list
 */
string t_php_generator::argument_list(t_struct* tstruct) {
  string result = "";

  const vector<t_field*>& fields = tstruct->get_members();
  vector<t_field*>::const_iterator f_iter;
  bool first = true;
  for (f_iter = fields.begin(); f_iter != fields.end(); ++f_iter) {
    if (first) {
      first = false;
    } else {
      result += ", ";
    }
    result += "$" + (*f_iter)->get_name();
  }
  return result;
}

/**
 * Gets a typecast string for a particular type.
 */
string t_php_generator::type_to_cast(t_type* type) {
  if (type->is_base_type()) {
    t_base_type* btype = (t_base_type*)type;
    switch (btype->get_base()) {
    case t_base_type::TYPE_BOOL:
      return "(bool)";
    case t_base_type::TYPE_BYTE:
    case t_base_type::TYPE_I16:
    case t_base_type::TYPE_I32:
    case t_base_type::TYPE_I64:
      return "(int)";
    case t_base_type::TYPE_DOUBLE:
      return "(double)";
    case t_base_type::TYPE_STRING:
      return "(string)";
    default:
      return "";
    }
  } else if (type->is_enum()) {
    return "(int)";
  }
  return "";
}

/**
 * Converts the parse type to a C++ enum string for the given type.
 */
string t_php_generator ::type_to_enum(t_type* type) {
  type = get_true_type(type);

  if (type->is_base_type()) {
    t_base_type::t_base tbase = ((t_base_type*)type)->get_base();
    switch (tbase) {
    case t_base_type::TYPE_VOID:
      throw "NO T_VOID CONSTRUCT";
    case t_base_type::TYPE_STRING:
      return "TType::STRING";
    case t_base_type::TYPE_BOOL:
      return "TType::BOOL";
    case t_base_type::TYPE_BYTE:
      return "TType::BYTE";
    case t_base_type::TYPE_I16:
      return "TType::I16";
    case t_base_type::TYPE_I32:
      return "TType::I32";
    case t_base_type::TYPE_I64:
      return "TType::I64";
    case t_base_type::TYPE_DOUBLE:
      return "TType::DOUBLE";
    }
  } else if (type->is_enum()) {
    return "TType::I32";
  } else if (type->is_struct() || type->is_xception()) {
    return "TType::STRUCT";
  } else if (type->is_map()) {
    return "TType::MAP";
  } else if (type->is_set()) {
    return "TType::SET";
  } else if (type->is_list()) {
    return "TType::LST";
  }

  throw "INVALID TYPE IN type_to_enum: " + type->get_name();
}

THRIFT_REGISTER_GENERATOR(php, "PHP",
"    inlined:         Generate PHP inlined files\n"
"    server:          Generate PHP server stubs\n"
"    autoload:        Generate PHP with autoload\n"
"    oop:             Generate PHP with object oriented subclasses\n"
"    rest:            Generate PHP REST processors\n"
"    namespace:       Generate PHP namespaces as defined in PHP >= 5.3\n"
)

