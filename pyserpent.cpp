#include <boost/python.hpp>
#include <Python.h>
#include "util.h"
#include "bignum.h"
#include "tokenize.h"
#include "parser.h"
#include "rewriter.h"
#include "compiler.h"
#include "lllparser.h"
#include <boost/python/stl_iterator.hpp>

using namespace boost::python;

std::string compile(std::string input) {
    return assemble(compile_lll(rewrite(parseSerpent(input))));
}

//http://stackoverflow.com/questions/5314319/how-to-export-stdvector
template<class T>
struct VecToList
{
    static PyObject* convert(const std::vector<T>& vec)
    {
        boost::python::list* l = new boost::python::list();
        for(size_t i = 0; i < vec.size(); i++)
            (*l).append(vec[i]);

        return l->ptr();
    }
};

//http://code.activestate.com/lists/python-cplusplus-sig/16463/
template<typename T>
struct Vector_from_python_list
{

    Vector_from_python_list()
    {
      using namespace boost::python;
      using namespace boost::python::converter;
      registry::push_back(&Vector_from_python_list<T>::convertible,
              &Vector_from_python_list<T>::construct,
              type_id<std::vector<T> 
>());

    }
 
    // Determine if obj_ptr can be converted in a std::vector<T>
    static void* convertible(PyObject* obj_ptr)
    {
      if (!PyList_Check(obj_ptr)){
    return 0;
      }
      return obj_ptr;
    }
 
    // Convert obj_ptr into a std::vector<T>
    static void construct(
    PyObject* obj_ptr,
    boost::python::converter::rvalue_from_python_stage1_data* data)
    {
      using namespace boost::python;
      // Extract the character data from the python string
      //      const char* value = PyString_AsString(obj_ptr);
      list l(handle<>(borrowed(obj_ptr)));

      // // Verify that obj_ptr is a string (should be ensured by convertible())
      // assert(value);
 
      // Grab pointer to memory into which to construct the new std::vector<T>
      void* storage = (
        (boost::python::converter::rvalue_from_python_storage<std::vector<T> 
>*)

        data)->storage.bytes;
 
      // in-place construct the new std::vector<T> using the character data
      // extraced from the python object
      std::vector<T>& v = *(new (storage) std::vector<T>());
 
      // populate the vector from list contains !!!
      int le = len(l);
      v.resize(le);
      for(int i = 0;i!=le;++i){
    v[i] = extract<T>(l[i]);
      }

      // Stash the memory chunk pointer for later use by boost.python
      data->convertible = storage;
    }
};

std::string printMetadata(Metadata m) { 
    return "["+m.file+" "+intToDecimal(m.ln)+" "+intToDecimal(m.ch)+"]";
}

Node compile_to_lll(std::string code) { return rewrite(parseSerpent(code)); }

std::vector<Node> pretty_assemble(Node code) {
     return flatten(dereference(code)); 
}

BOOST_PYTHON_FUNCTION_OVERLOADS(tokenize_overloads, tokenize, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(parse_overloads, parseSerpent, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(parselll_overloads, parseLLL, 1, 2);
BOOST_PYTHON_FUNCTION_OVERLOADS(printast_overloads, printAST, 1, 2);
//BOOST_PYTHON_FUNCTION_OVERLOADS(metadata_overloads, Metadata, 0, 3);
BOOST_PYTHON_MODULE(pyserpent)
{
    def("tokenize", tokenize, tokenize_overloads());
    def("parse", parseSerpent, parse_overloads());
    def("parseLLL", parseLLL, parselll_overloads());
    def("rewrite", rewrite);
    def("compile_to_lll", compile_to_lll);
    def("encode_datalist", static_cast<std::string (*)(std::vector<std::string>)>(&encodeDatalist));
    def("compile_lll", compile_lll);
    def("assemble", assemble);
    def("deserialize", deserialize);
    def("dereference", dereference);
    def("flatten", flatten);
    def("serialize", serialize);
    def("compile", compile);
    def("pretty_assemble", pretty_assemble);
    //class_<Node>("Node",init<>())
    to_python_converter<std::vector<Node,class std::allocator<Node> >,
                         VecToList<Node> >();
    Vector_from_python_list<Node>();
    Vector_from_python_list<std::string>();
    class_<Metadata>("Metadata",init<>())
        .def(init<std::string, int, int>())
        .def("__str__", printMetadata)
        .def("__repr__", printMetadata)
    ;
    class_<Node>("Node",init<>())
        .def(init<>())
        .def("__str__", printAST, printast_overloads())
        .def("__repr__", printAST, printast_overloads())
    ;
    //class_<Node>("Vector",init<>())
    //    .def(init<>());
}