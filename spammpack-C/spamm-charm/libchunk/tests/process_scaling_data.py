#!/usr/bin/env python

class scaling_data:

  def __init__ (self, filename):
    import re

    self.data = []
    self.N_chunk = 0
    self.N_basic = 0

    fd = open(filename)
    for line in fd:
      if self.N_chunk == 0:
        result = re.compile("^running.* -N ([0-9]+) -b ([0-9]*) ").search(line)
        if result:
          self.N_chunk = int(result.group(1))
          self.N_basic = int(result.group(2))

      if re.compile("running:").search(line):
        self.append()

      result = re.compile("lambda = (.*)$").search(line)
      if result:
        self.set_lambda(float(result.group(1)))

      result = re.compile("complexity ratio = (.*)$").search(line)
      if result:
        self.set_complexity(float(result.group(1)))

      result = re.compile("done multiplying using ([0-9]*) OpenMP.*tolerance (.*), (.*) seconds").search(line)
      if result:
        self.set_threads(int(result.group(1)))
        self.set_tolerance(float(result.group(2)))
        self.set_dense(False)
        self.set_walltime(float(result.group(3)))

      result = re.compile("done multiplying dense using ([0-9]*) OpenMP.* (.*) seconds").search(line)
      if result:
        self.set_threads(int(result.group(1)))
        self.set_dense(True)
        self.set_walltime(float(result.group(2)))

    fd.close()

  def info (self):
    info = ""
    info += "N = {:d}, N_basic = {:d}\n".format(self.N_chunk, self.N_basic)
    info += "complexity: " + str(self.get_complexity()) + "\n"
    info += "thread:     " + str(self.get_threads())
    return info

  def append (self):
    self.data.append({})

  def set_dense (self, isDense):
    self.data[-1]["isDense"] = isDense

  def set_lambda (self, l):
    self.data[-1]["lambda"] = l

  def set_complexity (self, c):
    self.data[-1]["complexity"] = c

  def set_threads (self, P):
    self.data[-1]["threads"] = P

  def set_tolerance (self, t):
    self.data[-1]["tolerance"] = t

  def set_walltime (self, T):
    self.data[-1]["walltime"] = T

  def __str__ (self):
    result = ""
    for i in self.data:
      result += str(i) + "\n"
    return result

  def get_complexity (self):
    c = []
    for i in self.data:
      if not i["complexity"] in c:
        c.append(i["complexity"])
    return sorted(c, reverse = True)

  def get_threads (self):
    t = []
    for i in self.data:
      if not i["threads"] in t:
        t.append(i["threads"])
    return sorted(t)

  def get_walltime (self, isDense = False, complexity = None, threads = None):
    result = []
    for i in self.data:
      next_result = i
      if complexity and i["complexity"] != complexity:
        next_result = None
      if threads and i["threads"] != threads:
        next_result = None
      if isDense != i["isDense"]:
        next_result = None
      if next_result:
        result.append(next_result)
    return result

def flatten_list (l):
  while True:
    temp = []
    isFlat = True
    for i in l:
      if type(i) == type([]):
        isFlat = False
        for j in i:
          temp.append(j)
      else:
        temp.append(i)
    l = temp
    if isFlat:
      break
  return l

def main ():
  import argparse
  import matplotlib.pyplot as plt

  parser = argparse.ArgumentParser()

  parser.add_argument("FILE",
      help = "The output file of the scaling script",
      nargs = "+")

  parser.add_argument("--no-title",
      help = "Do not add a title to the graphs",
      default = False,
      action = "store_true")

  parser.add_argument("--output",
      help = "Save figures into FILEBASE")

  parser.add_argument("--print",
      help = "Print the data file",
      default = False,
      action = "store_true")

  parser.add_argument("--thread",
      metavar = "N",
      help = "Plot only results for N threads",
      nargs = "+",
      action = "append")

  parser.add_argument("--complexity",
      metavar = "C",
      help = "Plot only results for complexity C",
      nargs = "+",
      action = "append")

  options = parser.parse_args()

  if options.thread:
    options.thread = flatten_list(options.thread)
    print("plotting only threads " + str(options.thread))

  if options.complexity:
    options.complexity = flatten_list(options.complexity)
    print("plotting only complexity " + str(options.complexity))

  for filename in options.FILE:
    data = scaling_data(filename)
    print(data.info())

    if options.print:
      print(str(data))

    # Plot walltime vs. complexity.
    figure1 = plt.figure()

    complexity_values = data.get_complexity()
    if options.thread:
      thread_values = sorted([ int(i) for i in options.thread ])
    else:
      thread_values = data.get_threads()

    for t in thread_values:
      walltime = []
      for c in complexity_values:
        query = data.get_walltime(complexity = c, threads = t)
        if len(query) != 1:
          raise Exception("can not find result for "
          + "complexity {:1.3f} and {:d} threads".format(c, t))
        walltime.append(query[0]["walltime"])
      plt.loglog(
          complexity_values,
          [ walltime[0]/i for i in walltime ],
          linestyle = "-",
          marker = "o",
          label = "{:d} threads".format(t)
          )

    plt.loglog(
        complexity_values,
        [ 1/i for i in complexity_values ],
        color = "black",
        label = "ideal"
        )

    plt.grid(True)
    plt.xlim([min(complexity_values), max(complexity_values)])
    plt.ylim([1/max(complexity_values), 1/min(complexity_values)])
    plt.gca().invert_xaxis()
    plt.legend(loc = "upper left")
    plt.xlabel("complexity")
    plt.ylabel("parallel speedup")
    if not options.no_title:
      plt.title("N = {:d}, N_basic = {:d}".format(data.N_chunk, data.N_basic))

    if options.output:
      plt.savefig(options.output + "_complexity.png")

    # Plot walltime vs. threads.
    figure2 = plt.figure()

    if options.complexity:
      complexity_values = sorted(
          [ float(i) for i in options.complexity ],
          reverse = True
          )
    else:
      complexity_values = data.get_complexity()
    thread_values = data.get_threads()

    for c in complexity_values:
      walltime = []
      for t in thread_values:
        query = data.get_walltime(complexity = c, threads = t)
        if len(query) != 1:
          raise Exception("can not find SpAMM result for {:d} threads".format(t))
        walltime.append(query[0]["walltime"])
      plt.loglog(
          thread_values,
          [ walltime[0]/i for i in walltime ],
          linestyle = "-",
          marker = "o",
          label = "complexity {:1.3f}".format(c)
          )

    walltime = []
    for t in thread_values:
      query = data.get_walltime(isDense = True, threads = t)
      if len(query) != 1:
        raise Exception("can not find dense result for {:d} threads".format(t))
      walltime.append(query[0]["walltime"])
    plt.loglog(
        thread_values,
        [ walltime[0]/i for i in walltime ],
        linestyle = "-",
        marker = "*",
        label = "dense"
        )

    plt.loglog(
        thread_values,
        thread_values,
        color = "black",
        label = "ideal"
        )

    plt.grid(True)
    plt.legend(loc = "upper left")
    plt.xlim([min(thread_values), max(thread_values)])
    plt.ylim([min(thread_values), max(thread_values)])
    plt.xlabel("threads")
    plt.ylabel("parallel speedup")
    if not options.no_title:
      plt.title("N = {:d}, N_basic = {:d}".format(data.N_chunk, data.N_basic))

    if options.output:
      plt.savefig(options.output + "_threads.png")

    # Plot parallel efficiency vs. threads.
    figure3 = plt.figure()

    if options.complexity:
      complexity_values = sorted(
          [ float(i) for i in options.complexity ],
          reverse = True
          )
    else:
      complexity_values = data.get_complexity()
    thread_values = data.get_threads()

    for c in complexity_values:
      walltime = []
      for t in thread_values:
        query = data.get_walltime(complexity = c, threads = t)
        if len(query) == 0:
          raise Exception("can not find SpAMM result for {:d} threads".format(t))
        walltime.append(query[0]["walltime"])
      plt.plot(
          thread_values,
          [ 100*walltime[0]/walltime[i]/thread_values[i] for i in range(len(walltime)) ],
          linestyle = "-",
          marker = "o",
          label = "complexity {:1.3f}".format(c)
          )

    walltime = []
    for t in thread_values:
      query = data.get_walltime(isDense = True, threads = t)
      if len(query) == 0:
        raise Exception("can not find dense result for {:d} threads".format(t))
      walltime.append(query[0]["walltime"])
    plt.plot(
        thread_values,
        [ 100*walltime[0]/walltime[i]/thread_values[i] for i in range(len(walltime)) ],
        linestyle = "-",
        marker = "*",
        label = "dense"
        )

    plt.grid(True)
    plt.legend(loc = "lower left")
    plt.xlabel("threads")
    plt.ylabel("parallel efficiency")
    if not options.no_title:
      plt.title("N = {:d}, N_basic = {:d}".format(data.N_chunk, data.N_basic))

    if options.output:
      plt.savefig(options.output + "_efficiency.png")

  plt.show()

if __name__ == "__main__":
  main()
