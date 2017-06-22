TEMPLATE = subdirs

SUBDIRS = \
        nemo-connectivity \
        plugin

plugin.depends = nemo-connectivity
