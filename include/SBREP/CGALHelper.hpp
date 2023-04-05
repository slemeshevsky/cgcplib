#ifndef CGALHELPER_H
#define CGALHELPER_H

#include <CGAL/Modifier_base.h>
#include <CGAL/Polyhedron_3.h>
#include <CGAL/Polyhedron_incremental_builder_3.h>
#include <CGAL/Timer.h>
#include <CGAL/exceptions.h>
#include <exception>
#include <fstream>
#include <string>

namespace SBREP {

using Group = std::vector<std::size_t>;
using Groups = std::vector<Group>;

template <class Refs> struct GFace : public CGAL::HalfedgeDS_face_base<Refs> {
  int groupId;
};

struct Polyhedron_items : public CGAL::Polyhedron_items_3 {
  template <class Refs, class Traits> struct Face_wrapper {
    typedef GFace<Refs> Face;
  };
};

namespace IO {
;

template <class HDS>
class BuildCGALPolyhedronFromObj : public CGAL::Modifier_base<HDS> {
public:
  BuildCGALPolyhedronFromObj(const std::string file_name)
      : m_fileName(file_name) {
    groups = Groups();
  }

  void operator()(HDS &hds) {
    typedef typename HDS::Vertex Vertex;
    typedef typename Vertex::Point Point;

    std::ifstream _file(m_fileName.c_str());
    if (!_file) {
      return;
    }

    std::string _line;
    std::cout << _line << std::endl;

    int _numVertices = 0;
    int _numFacets = 0;
    int _numGroups = 0;
    CGAL::Timer timer;

    timer.start();
    std::cout << "* Starting reading obj file" << std::endl;

    while (_file.good()) {
      std::getline(_file, _line);
      if (_line.size() > 1) {
        if (_line[0] == 'v' && _line[1] == ' ') {
          ++_numVertices;
        }
        if (_line[0] == 'f' && _line[1] == ' ') {
          ++_numFacets;
        }
        if (_line[0] == 'g' && _line[1] == ' ') {
          ++_numGroups;
        }
      }
    }

    if (!_file.good()) {
      _file.clear();
    }
    _file.seekg(0);

    CGAL::Polyhedron_incremental_builder_3<HDS> B(hds, true);
    groups.clear();
    groups.resize(_numGroups);

    B.begin_surface(_numVertices, _numFacets,
                    int(_numVertices + _numFacets - 2) * 2.1);
    std::string _token;
    int _groupId = -1;
    int _faceId = 0;
    while (!_file.eof()) {
      _token = "";
      _file >> _token;

      if (_token == "v") {
        double x, y, z;
        _file >> x >> y >> z;
        B.add_vertex(Point(x, y, z));
      } else if (_token == "g") {
        _groupId++;
      } else if (_token == "f") {
        std::string _line;
        std::getline(_file, _line);
        std::istringstream _stream(_line);
        std::vector<std::string> _vertices;
        std::copy(std::istream_iterator<std::string>(_stream),
                  std::istream_iterator<std::string>(),
                  std::back_inserter(_vertices));
        B.begin_facet();
        for (size_t i = 0; i < _vertices.size(); ++i) {
          std::string::size_type _pos = _vertices[i].find('/', 0);
          std::string _indexStr = _vertices[i].substr(0, _pos);
          B.add_vertex_to_facet(stoi(_indexStr) - 1);
        }
        auto h = B.end_facet();
        if (_numGroups > 0)
          groups[_groupId].push_back(_faceId);
        ++_faceId;
        //        h->facet()->groupId = _groupId;
      }
    }
    _file.close();
    B.end_surface();
    timer.stop();
    std::cout << "* Read file within " << timer.time() << " seconds."
              << std::endl;
    std::cout << "Number of Vertices: " << _numVertices << std::endl;
    std::cout << "Number of Faces: " << _numFacets << std::endl;
    std::cout << "Number of Groups: " << _numGroups << std::endl;
    timer.reset();
  };

  Groups get_groups() { return groups; }

private:
  std::string m_fileName;
  Groups groups;
};

template <class TPoly>
std::pair<TPoly, Groups> read_obj(const std::string file_name) {
  TPoly mesh;
  Groups groups;

  try {
    BuildCGALPolyhedronFromObj<typename TPoly::HalfedgeDS> _buildPolyhedron(
        file_name);
    mesh.delegate(_buildPolyhedron);
    //    std::string filename = CGAL::data_file_path(file_name);
    if (!mesh.is_valid()) {
      throw CGAL::Assertion_exception("", "", "", 0, "");
    };
    groups = _buildPolyhedron.get_groups();
  } catch (CGAL::Assertion_exception &) {
    std::string _msg =
        "SBREP::Builder::ReadObjFile: Error loading " + file_name;
    throw std::exception();
  }
  std::pair<TPoly, Groups> res{mesh, groups};
  return res;
}
}; // namespace IO
}; // namespace SBREP

#endif /* CGALHELPER_H */
