#include <stdlib.h>
#include <iostream>
#include <fstream>
#include <sstream>      // std::stringstream
#include <vector>
#include <cstddef>


struct TriangleMesh
{
    int vertex_count;
    double * vertex_coords;

    int triangle_count;

    // The traingle vertices are stored in groups of four indices,
    // first three are indices of the vertices from mesh->vertex_coords,
    // the fourth is the index of the normal.
    int * triangle_vertices;

    // One are more triangle can have same normal given by the same index
    // Case with mutiple loops for each facet in the STL file.
    double * normal_coords;
};


#define SUCCESS 0
#define ARGS_ERROR 1
#define MEMORY_ERROR 2
#define FILEREAD_ERROR 3
#define FILEFORMAT_INCORRECT 4
// TODO: more error codes

typedef int ERROR_TYPE;

//Exception Safety - nothrow guarantee
ERROR_TYPE init_mesh(TriangleMesh * mesh)
{
    mesh->vertex_count = 0;
    mesh->vertex_coords = NULL;

    mesh->triangle_count = 0;

    mesh->triangle_vertices = NULL;
    mesh->normal_coords = NULL;
    return 0;
}

//Exception safety - no throw guarantee
ERROR_TYPE free_mesh(TriangleMesh * mesh)
{
    // TODO: implement
    mesh->vertex_count=0;
    delete []mesh->vertex_coords;

    mesh->triangle_count = 0;
    delete []mesh->triangle_vertices;
    delete []mesh->normal_coords;

    return 0;
}

ERROR_TYPE read_STL(const char * filename, TriangleMesh * mesh)
{
    // TODO: implement for ASCII STL files
    // https://en.wikipedia.org/wiki/STL_%28file_format%29
    // files for testing: http://people.sc.fsu.edu/~jburkardt/data/stla/stla.html

    //pass over the file

    std::ifstream file(filename);
    std::string name_solid = ""; // name of the solid

    int facets=-1;  // the (number of facet-1) being processed;

    int vertex_count =0;  //temp variable for vertex_count
    std::vector<double> vertex_coords; // temp dynamicstorage for vertices
    std::vector<int> triangle_vertices; //temp dynamic storage for triangle_vertices
    int triangle_count=0;
    int normal_count=0;
    std::vector<double> normal_coords;  //temp dynamic storage for normal_coords

    if(file.is_open())
    {
        //current line number
        int line_number =0;
        // current_lineloop gices the loop we are inside in
        // current_lineloop is equal to 1 for solid, 2 for facet, 3 for loop, 0 for none;
        int current_lineloop = 0;

        std::string line;
        while(std::getline(file,line))
        {
            line_number++;
            std::stringstream ss(line);
            std::string line_type;
            std::string line_type2;
            line_type = ""; line_type2 = "";
            ss >> line_type;
            // for finding empty line
            std::size_t found = line_type.find_last_of(" ");
            line_type = line_type.substr(found+1);
            if(line_type.substr(0,2)=="//" || line_type==""){
                continue;  //a commented line detected or an empty line with whitespaces
            }
            else if(line_type == "solid")
            {
                ss >> name_solid;
                if(current_lineloop != 0){
                    std::cout <<"line_number : "<<line_number <<"\n";
                    return FILEFORMAT_INCORRECT;
                }
                current_lineloop = 1;
            }
            else if(line_type == "endsolid")
            {
                if(current_lineloop != 1){
                    std::cout <<"line_number : "<<line_number <<"\n";
                    return FILEFORMAT_INCORRECT;
                }
                // the mesh region has ended;
                current_lineloop = 0;
            }
            else if(line_type == "facet")
            {
                if(current_lineloop != 1){
                    std :: cout << current_lineloop<<"exp\n";
                    std::cout <<"line_number : "<<line_number <<"\n";
                    return FILEFORMAT_INCORRECT;
                }
                // start of a new facet (or) triangle
                //empty facets are possible - should take care
                facets++;
                ss >> line_type2;
                if(line_type2=="normal")
                {
                    double x,y,z;
                    ss >>x >>y >>z;
                    normal_coords.push_back(x); normal_coords.push_back(y); normal_coords.push_back(z);
                    normal_count++;
                }
                else{
                  std::cout <<"line_number : "<<line_number <<"\n";
                  return FILEFORMAT_INCORRECT;
                }

                current_lineloop = 2;
            }
            else if(line_type == "endfacet")
            {
                if(current_lineloop != 2){
                    std::cout <<"line_number : "<<line_number <<"\n";
                    return FILEFORMAT_INCORRECT;
                }
                //end of the tiangle
                current_lineloop = 1;
            }
            else if(line_type == "outer"){
                ss >> line_type2;
                if(current_lineloop != 2){
                    std::cout <<"line_number : "<<line_number <<"\n";
                    return FILEFORMAT_INCORRECT;
                }
                if(line_type2=="loop")
                {
                    // loop of vertices has started
                    current_lineloop = 3;
                }
                else{
                  std::cout <<"line_number : "<<line_number <<"\n";
                  return FILEFORMAT_INCORRECT;
                }

            }
            else if(line_type == "endloop")
            {
                if(current_lineloop != 3){
                    std::cout <<"line_number : "<<line_number <<"\n";
                    return FILEFORMAT_INCORRECT;
                }
                try{
                    //adding the index of the normal vertex to the triangle.
                    triangle_vertices.push_back(normal_count-1);
                    triangle_count++;
                }
                catch(std::bad_alloc &e){
                    std::cout << "vertices push_back error\n";
                    return MEMORY_ERROR;
                }
                current_lineloop = 2;
            }
            else if(line_type == "vertex")
            {
                if(current_lineloop != 3){
                    std::cout <<"line_number : "<<line_number <<"\n";
                    return FILEFORMAT_INCORRECT;
                }
                double x;double y; double z;
                ss >> x >>y >>z;
                //check if the vertex is already found
                bool found =0;
                // i - index of vertex
                int i;
                for(i=0;i<vertex_count;i++)
                {
                    if(vertex_coords[3*i]==x && vertex_coords[3*i+1]==y && vertex_coords[3*i+2]==z)
                    {
                        // vertex already detected
                        found=1;
                        break;
                    }
                }
                //i = vertex_count  if not "found"
                try{
                    //adding the indices of the vertices to the triangle.
                    triangle_vertices.push_back(i);
                }
                catch(std::bad_alloc &e){
                    std::cout << "vertices push_back error\n";
                    return MEMORY_ERROR;
                }

                if(!found)
                {
                    try{
                        vertex_coords.push_back(x);vertex_coords.push_back(y);vertex_coords.push_back(z);
                    }
                    catch(std::bad_alloc &e){
                        std::cout << "vertices push_back error\n";
                        return MEMORY_ERROR;
                    }
                    vertex_count++;
                }
            }
            else{
                std::cout <<"line_number : "<<line_number <<"\n";
                return FILEFORMAT_INCORRECT;
            }

        }

    }
    else {
        std::cout << "Error opening file 2nd pass";
        return FILEREAD_ERROR;
    }

    mesh->triangle_count = triangle_count;
    mesh->vertex_count = vertex_count;

    try{
        mesh->triangle_vertices=new int[4*mesh->triangle_count];
    }
    catch(std::bad_alloc&){
        std :: cout << "error providing memory to mesh->triangle_vertices "<<"\n";
        return MEMORY_ERROR;
    }
    try{
        mesh->normal_coords = new double[3*normal_count];
    }
    catch(std::bad_alloc&){
        std :: cout << "error providing memory to mesh->normal_coords "<<"\n";
        return MEMORY_ERROR;
    }
    try{
        mesh->vertex_coords=new double[3*mesh->vertex_count];
    }
    catch(std::bad_alloc&){
        std :: cout << "error providing memory to mesh->vertex_coords "<<"\n";
        return MEMORY_ERROR;
    }

    for(int i=0;i<mesh->triangle_count*4;i++){
        // cout << vertices
        mesh->triangle_vertices[i]=triangle_vertices[i]; //mesh->vertex_count is equal to vertices->size;
    }

    for(int i=0;i<normal_count*3;i++){
        // cout << vertices
        mesh->normal_coords[i]=normal_coords[i]; //mesh->vertex_count is equal to vertices->size;
    }

    for(int i=0;i<mesh->vertex_count*3;i++){
        // cout << vertices
        mesh->vertex_coords[i]=vertex_coords[i]; //mesh->vertex_count is equal to vertices->size;
    }

    //file is read and datadtructures initiated (or) the file is empty.
    return SUCCESS;
}


int main(int argc, char *argv[])
{
  // TODO: handle input parameters argc and argv
  // TODO: error handling

  if(argc != 2){
      //incorrect number of parameters entered;
      std::cout << "incorrect number of args entered" << std::endl;
      //for failure detection;
      return ARGS_ERROR;
  }

  TriangleMesh mesh;
  init_mesh(&mesh);
  int error = read_STL(argv[1], &mesh);

  if(error == MEMORY_ERROR){
      //reaches here in case of bad_alloc& error
      std::cout <<"allot more memory\n";
      free_mesh(&mesh);
      return MEMORY_ERROR;
  }
  if(error == FILEREAD_ERROR){
      //reaches here in case of file read error
      std::cout << "check for the file in the args\n";
      free_mesh(&mesh);
      return FILEREAD_ERROR;
  }
  if(error == FILEFORMAT_INCORRECT){
      //reaches here in case of incorrect file format error
      std::cout << "Incorrect file format\n";
      free_mesh(&mesh);
      return FILEFORMAT_INCORRECT;
  }

  std::cout <<"number_of_vertices = "<< mesh.vertex_count<<"\n";
  std::cout <<"number_of_triangles = "<< mesh.triangle_count<<"\n";
  free_mesh(&mesh);
  std::cout << "after_deletion"<<"\n";
  std::cout <<"number_of_vertices = "<< mesh.vertex_count<<"\n";
  std::cout <<"number_of_triangles = "<< mesh.triangle_count<<"\n";

  //The program successfully completed.
  return SUCCESS;
}
