classdef JointDOFsDoubleArray < iDynTree.VectorDynSize
  methods
    function self = JointDOFsDoubleArray(varargin)
      self@iDynTree.VectorDynSize(SwigRef.Null);
      if nargin==1 && strcmp(class(varargin{1}),'SwigRef')
        if varargin{1}~=SwigRef.Null
          self.swigPtr = varargin{1}.swigPtr;
        end
      else
        tmp = iDynTreeMEX(860, varargin{:});
        self.swigPtr = tmp.swigPtr;
        tmp.swigPtr = [];
      end
    end
    function varargout = resize(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(861, self, varargin{:});
    end
    function varargout = isConsistent(self,varargin)
      [varargout{1:nargout}] = iDynTreeMEX(862, self, varargin{:});
    end
    function delete(self)
      if self.swigPtr
        iDynTreeMEX(863, self);
        self.swigPtr=[];
      end
    end
  end
  methods(Static)
  end
end