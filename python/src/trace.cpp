#include <qipython/trace.hpp>
#include <qipython/gil.hpp>
#include <qipython/error.hpp>
#include <boost/foreach.hpp>
#include <boost/python/stl_iterator.hpp>

qiLogCategory("qipython");

namespace bpy = boost::python;

namespace qi {
  namespace py {
    template <typename T>
    static T pyTo(bpy::object o)
    {
      // we need an explicit cast in most cases
      return T(bpy::extract<T>(o));
    }

    QIPYTHON_API std::string thread_traces()
    {
      GILScopedLock lock;
      try
      {
        bpy::object sys = bpy::import("sys");
        bpy::object traceback = bpy::import("traceback");
        bpy::object inspect = bpy::import("inspect");

        std::ostringstream out;

        // for each tuple (threadid, stack)
        for (bpy::stl_input_iterator<bpy::tuple> iter =
            bpy::dict(sys.attr("_current_frames")()).items();
            iter != bpy::stl_input_iterator<bpy::tuple>();
            ++iter)
        {
          out << "\n# ThreadID: " << ((uint64_t)pyTo<int64_t>((*iter)[0])) << "\n";

          // for each frame in the stack which is a tuple
          // (frame, file, line, function, list(code))
          for (bpy::stl_input_iterator<bpy::tuple> iter2 =
                inspect.attr("getouterframes")(bpy::object((*iter)[1]), 5);
              iter2 != bpy::stl_input_iterator<bpy::tuple>();
              ++iter2)
          {
            out << "  File " << pyTo<std::string>((*iter2)[1])
              << ", line " << pyTo<int>((*iter2)[2])
              << ", in " << pyTo<std::string>((*iter2)[3]);
            bpy::list args(inspect.attr("getargvalues")(
                bpy::object((*iter2)[0])));
            out << pyTo<std::string>(
                inspect.attr("formatargvalues")(
                  bpy::object(args[0]),
                  bpy::object(args[1]),
                  bpy::object(args[2]),
                  bpy::object(args[3])));
            out << '\n';

            if (!bpy::object((*iter2)[4]).is_none())
            {
              // for each line of context
              unsigned int i = 0;
              for (bpy::stl_input_iterator<std::string> iter3 =
                    bpy::list((*iter2)[4]);
                  iter3 != bpy::stl_input_iterator<std::string>();
                  ++iter3, ++i)
              {
                if (i == pyTo<unsigned int>((*iter2)[5]))
                  out << " => " << *iter3;
                else
                  out << "    " << *iter3;
              }
              out << '\n';
            }
          }
        }

        qiLogVerbose("qipython.debug") << "Traces:\n" << out.str();
        return out.str();
      }
      catch (bpy::error_already_set& e)
      {
        qiLogError() << "Cannot get trace: " << PyFormatError();
        return "";
      }
    }
  }
}
