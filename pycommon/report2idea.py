import sys, os.path
sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "..", "nemea-framework", "python"))
import argparse
import json
import trap
import unirec
from time import time, gmtime
from uuid import uuid4
from datetime import datetime

def getRandomId():
    """Return unique ID of IDEA message. It is done by UUID in this implementation."""
    return str(uuid4())

def setAddr(idea_field, addr):
    """Set IP address into 'idea_field'.
This method automatically recognize IPv4 vs IPv6 and sets the correct information into the IDEA message.
Usage: setAddr(idea['Source'][0], rec.SRC_IP)"""

    if isinstance(addr, unirec.ur_ipaddr.IP6Addr):
        idea_field['IP6'] = [str(addr)]
    else:
        idea_field['IP4'] = [str(addr)]

def getIDEAtime(unirecField = None):
    """Return timestamp in IDEA format (string).
    If unirecField is provided, it will convert it into correct format.
    Otherwise, current time is returned."""

    if unirecField:
        return unirecField.toString('%Y-%m-%dT%H:%M:%SZ')
    else:
        t = time()
        g = gmtime(t)
        iso = '%04d-%02d-%02dT%02d:%02d:%02dZ' % g[0:6]
    return iso


# TODO: resolve argument parsing and help in Python modules
# Ideally it should all be done in Python using overloaded ArgParse

# TODO: allow setting library verbose mode

# Template of module description
desc_template = """
TRAP module, libtrap version: [TODO]
===========================================
Name: {name}
Inputs: 1
Outputs: 0 or 1 (depending on --trap parameter)
Description:
  {original_desc}Required format of input:
    {type}: "{fmt}"
  
  All '<something>2idea' modules convert reports from various detectors to Intrusion Detection Extensible Alert (IDEA) format. The IDEA messages may be send to any of the following outputs:
    - TRAP interface (--trap)
    - simple text file (--file)
    - collection in MongoDB database (--mongodb) 
    - Warden3 server (--warden)
  It is possible to define more than one outputs - the messages will be send to all of them.
"""

DEFAULT_NODE_NAME = "undefined"

def Run(module_name, module_desc, req_type, req_format, conv_func, arg_parser = None):
    """ TODO doc
    """
    
    # *** Parse command-line arguments ***
    if arg_parser is None:
       arg_parser = argparse.ArgumentParser()
    arg_parser.formatter_class = argparse.RawDescriptionHelpFormatter

    # Set description
    arg_parser.description = str.format(desc_template,
        name=module_name,
        type={trap.TRAP_FMT_RAW:'raw', trap.TRAP_FMT_UNIREC:'UniRec', trap.TRAP_FMT_JSON:'JSON'}.get(req_type,'???'),
        fmt=req_format,
        original_desc = module_desc+"\n\n  " if module_desc else "",
    )
    
    # Add arguments defining outputs
    # TRAP output
    arg_parser.add_argument('--trap', action='store_true',
                            help='Enable output via TRAP interface (JSON type with format id "IDEA"). Parameters are set using "-i" option as usual.')
    # File output
    arg_parser.add_argument('--file', metavar="FILE", type=str,
                            help='Enable output to file (each IDEA message printed to new line in JSON format). Set to "-" to use standard output.')
    arg_parser.add_argument('--file-indent', metavar="N", type=int,
                            help='Pretty-format JSON in output file using N spaces for indentation.')
    arg_parser.add_argument('--file-append', action='store_true',
                            help='Append to file instead of overwrite.')
    # MongoDB output
    arg_parser.add_argument('--mongodb', metavar="DBNAME",
                            help='Enable output to MongoDB. Connect to database named DBNAME.')
    arg_parser.add_argument('--mongodb-coll', metavar="COLL", default='alerts',
                            help='Put IDEA messages into collection named COLL (default: "alerts").')
    arg_parser.add_argument('--mongodb-host', metavar="HOSTNAME", default='localhost',
                            help='Connect to MongoDB running on HOSTNAME (default: "localhost").')
    arg_parser.add_argument('--mongodb-port', metavar="PORT", type=int, default=27017,
                            help='Connect to MongoDB running on port number PORT (default: 27017).')
    # Warden3 output
    arg_parser.add_argument('--warden', metavar="CONFIG_FILE",
                            help='Send IDEA messages to Warden server. Load configuration of Warden client from CONFIG_FILE.')
    
    # Other options
    arg_parser.add_argument('-n', '--name', metavar='NODE_NAME',
                            help='Name of the node, filled into "Node.Name" element of the IDEA message. Required if Warden output is used, recommended otherwise.')
    arg_parser.add_argument('--test', action='store_true',
                            help='Add "Test" to "Category" before sending a message to output(s).')
    arg_parser.add_argument('-v', '--verbose', action='store_true',
                            help="Enable verbose mode (may be used by some modules, common part donesn't print anything")
    
    # TRAP parameters                       
    trap_args = arg_parser.add_argument_group('Common TRAP parameters')
    trap_args.add_argument('-i', metavar="IFC_SPEC", required=True,
                           help='TODO (ideally this section should be added by TRAP')
    
    
    # Parse arguments
    args = arg_parser.parse_args()
    
    # Check if at least one output is enabled
    if not (args.file or args.trap or args.mongodb or args.warden):
        sys.stderr.write(module_name+": Error: At least one output must be selected\n")
        exit(1)
    
    # Check if node name is set if Warden output is enabled
    if args.name is None:
        if args.warden:
            sys.stderr.write(module_name+": Error: Node name must be specified if Warden output is used (set param --name).\n")
            exit(1)
        else:
            sys.stderr.write(module_name+": Warning: Node name is not specified.\n")
    
    # *** Initialize TRAP ***
    
    module_info = trap.CreateModuleInfo(
        module_name, # Module name
        "", # Description
        1, # Number of input interfaces
        1 if args.trap else 0, # Number of output interfaces
        None # optionParser
    )
    ifc_spec = trap.parseParams(['-i', args.i], module_info)
    trap.init(module_info, ifc_spec)
    trap.registerDefaultSignalHandler()

    # Set required input format
    trap.set_required_fmt(0, req_type, req_format)

    # If TRAP output is enabled, set output format (JSON, format id "IDEA")
    if args.trap:
        trap.set_data_fmt(0, trap.TRAP_FMT_JSON, "IDEA")
    
    # *** Create output handles/clients/etc ***
    filehandle = None
    mongoclient = None
    mongocoll = None
    wardenclient = None
    
    if args.file:
        if args.file == '-':
            filehandle = sys.stdout
        else:
            filehandle = open(args.file, "a" if args.file_append else "w")
    
    if args.mongodb:
        import pymongo
        mongoclient = pymongo.MongoClient(args.mongodb_host, args.mongodb_port)
        mongocoll = mongoclient[args.mongodb][args.mongodb_coll]
    
    if args.warden:
        import warden_client
        wardenclient = warden_client.Client(**warden_client.read_cfg(args.warden))
    
    
    # *** Main loop ***   
    URInputTmplt = None
    if req_type == trap.TRAP_FMT_UNIREC and req_format != "":
        URInputTmplt = unirec.CreateTemplate("URInputTmplt", req_format) # TRAP expects us to have predefined template for required set of fields 
    
    while not trap.stop:
        # *** Read data from input interface ***
        try:
            data = trap.recv(0)
        except trap.EFMTMismatch:
            sys.stderr.write(module_name+": Error: input data format mismatch\n")#Required: "+str((req_type,req_format))+"\nReceived: "+str(trap.get_data_fmt(trap.IFC_INPUT, 0))+"\n")
            break
        except trap.EFMTChanged as e:
            # TODO: This should be handled by trap.recv transparently
            # Get negotiated input data format
            (fmttype, fmtspec) = trap.get_data_fmt(trap.IFC_INPUT, 0)
            # If data type is UniRec, create UniRec template
            #print "Creating template", fmtspec
            if fmttype == trap.TRAP_FMT_UNIREC:
                URInputTmplt = unirec.CreateTemplate("URInputTmplt", fmtspec)
            else:
                URInputTmplt = None
            data = e.data
        except trap.ETerminated:
            break
    
        # Check for "end-of-stream" record
        if len(data) <= 1:
            # If we have output, send "end-of-stream" record and exit
            if args.trap:
                trap.send(0, "0")
            break
    
        # Assert that if UniRec input is required, input template is set
        assert(req_type != trap.TRAP_FMT_UNIREC or URInputTmplt is not None)
        
        # Convert raw input data to UniRec object (if UniRec input is expected)
        if req_type == trap.TRAP_FMT_UNIREC:
            rec = URInputTmplt(data)
        elif req_type == trap.TRAP_FMT_JSON:
            rec = json.loads(data)
        else: # TRAP_FMT_RAW
            rec = data
        
        # *** Convert input record to IDEA ***
        
        # Pass the input record to conversion function to create IDEA message
        idea = conv_func(rec, args)
        
        if idea is None:
            continue # Record can't be converted - skip it (notice should be printed by the conv function)
        
        if args.name is not None:
            idea['Node'][0]['Name'] = args.name
        
        if args.test:
            idea['Category'].append('Test')
        
        # *** Send IDEA to outputs ***
        
        # File output
        if filehandle:
            filehandle.write(json.dumps(idea, indent=args.file_indent)+'\n')
        
        # TRAP output
        if args.trap:
            try:
                trap.send(0, json.dumps(idea))
            except trap.ETerminated:
                # don't exit immediately, first finish sending to other outputs
                trap.stop = 1

        # MongoDB output
       
        if mongocoll:
            # Convert timestamps from string to Date format
            idea['DetectTime'] = datetime.strptime(idea['DetectTime'], "%Y-%m-%dT%H:%M:%SZ")
            for i in [ 'CreateTime', 'EventTime', 'CeaseTime' ]:
                if idea.has_key(i):
                    idea[i] = datetime.strptime(idea[i], "%Y-%m-%dT%H:%M:%SZ")

            try:
                mongocoll.insert(idea)
            except pymongo.errors.AutoReconnect:
                sys.stderr.write(module_name+": Error: MongoDB connection failure.\n")
                trap.stop = 1
        
        # Warden output
        if wardenclient:
            wardenclient.sendEvents([idea])
        

    # *** Cleanup ***
    if filehandle and filehandle != sys.stdout:
        filehandle.close()
    if mongoclient:
        mongoclient.close()
    if wardenclient:
        wardenclient.close()
    trap.finalize()

