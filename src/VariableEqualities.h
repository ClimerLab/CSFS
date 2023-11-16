#ifndef VARIABLE_EQUALITIES_H
#define VARIABLE_EQUALITIES_H

#include <set>
#include <sstream>
#include <vector>

class VariableEqualities
{
  private:
    std::vector<std::set<std::size_t> > sets;

  public:
    bool add(const std::size_t, const std::size_t);
    void clear();
    bool empty() const;
    bool exists(const std::size_t) const;
    bool exists(const std::size_t, const std::size_t) const;
    std::vector<std::vector<std::size_t> > get2dVector() const;
    std::string getEqualitiesString() const;
    void merge(const VariableEqualities &);
};

#endif

