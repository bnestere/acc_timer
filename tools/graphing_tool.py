import os, sys, time, random, re
import numpy as np
import matplotlib.pyplot as plt
from matplotlib.pyplot import figure
import matplotlib
from collections import Counter
import collections


def graphing(names, vals, kernel, avg_or_total):

    # data to plot
    number_groups = len(names)

    # create plot
    # fig, ax = plt.subplots()
    f = figure(figsize=(100,40))

    #names = unop_names

    index = np.arange(number_groups)
    bar_width = 0.2
    opacity = 0.8

    #the min/max are determined manually, this must change
    axes = plt.gca()
    #axes.set_xlim([xmin,xmax])
    #axes.set_ylim([2000000,15000000])

    if avg_or_total == 'total':
        bar1 = plt.bar(index, vals, bar_width, alpha = opacity, color = 'g', label = 'Accumulation Unoptimized')
        #bar2 = plt.bar(index + bar_width, op_vals, bar_width, alpha = opacity, color='b', label = 'Accumulation Optimized')
        
        plt.xlabel('Region Name', fontsize=80)
        plt.ylabel('Time (as a Percentage)', fontsize=80)
        plt.title('Average Global Accumulation', fontsize=80)
        plt.xticks(index + bar_width/2, names, fontsize=60)
        plt.yticks(fontsize=80)
        plt.legend(fontsize=80)

        plt.tight_layout()
        #plt.show()
        
    elif avg_or_total == 'average':
        bar1 = plt.bar(index, vals, bar_width, alpha=opacity, color = 'g', label = 'Instance Unoptimized')
        #bar2 = plt.bar(index + bar_width, op_vals, bar_width, alpha = opacity, color = 'b', label = 'Instance Optimized')

        plt.xlabel('Region Name', fontsize=80)
        plt.ylabel('Time (in Nanoseconds)', fontsize=80)
        plt.title('Average time', fontsize=80)
        plt.xticks(index + bar_width/2, names, fontsize=60)
        plt.yticks(fontsize=80)
        plt.legend(fontsize=80)

        plt.tight_layout()
        #plt.show()

    f.savefig("%s_%s.jpg"%(kernel, avg_or_total), bbox_inches='tight')
        

def sort_vals(alist):
    def atoi(text):
        return int(text) if text.isdigit() else text

    
    def natural_keys(text):
#         alist.sort(key=natural_keys) sorts in human order
#         http://nedbatchelder.com/blog/200712/human_sorting.html
#         (See Toothy's implementation in the comments)
        return [ atoi(c) for c in re.split('(\d+)', text) ]


    alist.sort(key=natural_keys)
    return (alist)
    

def get_avgs(values, count, avg_or_total):
    if avg_or_total == 'average':
        for val in values:

            values[val] = int(float(values[val]) / (count[val])) 

        return values
        
    elif avg_or_total == 'total':
        #the division by 100 is so the percent will not be a decimal value
        total_program_time = int(float(values['totalprogramtime']) / 100)
        for val in values:

            values[val] = (float(values[val]) / (count[val])) / total_program_time

        return values


def get_arrs(dictionary): 
    sorted_loop_names = sort_vals(sorted(dictionary.keys()))
    sorted_loop_vals = []
    del sorted_loop_names[-1]
    for loop in sorted_loop_names:
        sorted_loop_vals.append(dictionary[loop])
        
    return sorted_loop_names, sorted_loop_vals
        
#     this wont get me what I want because the sort will never be correct on two lists created this way
#     keys, values = zip(*dictionary.items())
#     print(keys)
#     print(values)


def get_dicts(arr_of_lines,kernel):
    dict_of_avgs = {}
    dict_of_totals = {}
    dict_of_counts = {}
    
    for line in arr_of_lines:
        result = re.search('(.*)\(Total, Average: ([0-9]+), ([0-9]+)\)', line)
        if result:
            #dictionary of averages
            if not (result.group(1)) in dict_of_avgs:
                dict_of_avgs[(result.group(1))] = int(result.group(3))
            else:
                dict_of_avgs[(result.group(1))] += int(result.group(3)) 

            #dictionary of totals
            if not (result.group(1)) in dict_of_totals:
                dict_of_totals[(result.group(1))] = int(result.group(2))
            else:
                dict_of_totals[(result.group(1))] += int(result.group(2)) 

            #dictionary of counts
            if not (result.group(1)) in dict_of_counts:
                dict_of_counts[(result.group(1))] = 1
            else:
                dict_of_counts[(result.group(1))] += 1

        if line.strip().startswith("Global"):
            break
            
    avgs_dict = get_avgs(dict_of_avgs, dict_of_counts, 'average')
    totals_dict = get_avgs(dict_of_totals, dict_of_counts, 'total')
    
    avgs_names, avgs_vals = get_arrs(avgs_dict)
    totals_names, totals_vals = get_arrs(totals_dict)
    
    return avgs_names, avgs_vals, totals_names, totals_vals
            
        
def get_time_data(kernel):  
    arr_of_lines_op = []
    
    with open("%s.timings.out"%(kernel), "r")as f:
        timing_lines = f.readlines()    
        
    avgs_names, avgs_val, totals_names, totals_val = get_dicts(timing_lines,kernel )

    graphing(avgs_names, avgs_val, kernel, "average")
    graphing(totals_names, totals_val, kernel, "total")

    print("\n")
            
        
if __name__ == '__main__':

    if(len(sys.argv) < 2):
        print("Usage: ./%s <benchmark_name>"%(sys.argv[0]))
        exit(0)

    get_time_data(sys.argv[1]);
    
#    kernels = ["radix", "lu_cb", "fft"]
#    for kernel in kernels:
#        number_of_threads = [1,2,4,8]
#        for thread_number in number_of_threads:
#            get_time_data(kernel, thread_number)  
            
    print("done")
            
