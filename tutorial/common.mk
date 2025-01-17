#
# automake template
#
# added by tim blechmann
# modified by grrrr.org
#
# this is being included by subfolder Makefiles

BUILT_SOURCES = main.cpp

CXXFLAGS  = @CXXFLAGS@ \
	@OPT_FLAGS@ \
	$(patsubst %,-I%,@INCLUDEDIRS@) \
	-I../../source \
    -D FLEXT_INLINE=1 \
	$(DEFS) \
	-DFLEXT_SHARED

LDFLAGS = @LD_FLAGS@ $(patsubst %,-L%,@LIBDIRS@)  $(patsubst %,-framework %,$(FRAMEWORKS))

FRAMEWORKS = @FRAMEWORKS@

TARGET = $(NAME).@EXTENSION@

OBJECTS = $(patsubst %.cpp,./%.@OBJEXT@,$(BUILT_SOURCES))

SYSDIR = @SYSDIR@


# ----------------------------- targets --------------------------------

all-local: $(OBJECTS)
	$(CXX) $(LDFLAGS) ./*.@OBJEXT@ $(LIBS) -o ./$(TARGET)
	strip -x ./$(TARGET)

./%.@OBJEXT@ : %.cpp 
	$(CXX) -c $(CXXFLAGS) $< -o $@

clean-local:
	rm -f ./$(TARGET)
	rm -f ./$(OBJECTS)

install-exec-local:
	install ./$(TARGET) $(SYSDIR)/extra
