(function (global, factory) {
  typeof exports === 'object' && typeof module !== 'undefined' ? factory(require('cesium')) :
  typeof define === 'function' && define.amd ? define(['cesium'], factory) :
  (global = global || self, factory(global.Cesium));
}(this, (function (cesium) { 'use strict';

  function styleInject(css, ref) {
    if ( ref === void 0 ) ref = {};
    var insertAt = ref.insertAt;

    if (!css || typeof document === 'undefined') { return; }

    var head = document.head || document.getElementsByTagName('head')[0];
    var style = document.createElement('style');
    style.type = 'text/css';

    if (insertAt === 'top') {
      if (head.firstChild) {
        head.insertBefore(style, head.firstChild);
      } else {
        head.appendChild(style);
      }
    } else {
      head.appendChild(style);
    }

    if (style.styleSheet) {
      style.styleSheet.cssText = css;
    } else {
      style.appendChild(document.createTextNode(css));
    }
  }

  var css_248z = ".cesium-measure-toolbar {\n    width: 40px;\n    height: 32px;\n    overflow: hidden;\n    transition: width 0.8s;\n    white-space: nowrap;\n    background: #303336;\n    border: 1px solid #444;\n    color: #edffff;\n    stroke: #edffff;\n    stroke-width: 2;\n    fill: transparent;\n}\n.cesium-measure-toolbar.expanded {\n    width: 423px;\n}\n.cesium-measure-button {\n    vertical-align: top;\n    display: inline-block;\n    border-right: 1px solid #444;\n    cursor: pointer;\n    padding: 4px 8px;\n    height: 25px;\n    width: 25px;\n    margin-left: -4px /*removes space between buttons*/\n}\n.cesium-measure-button:first-child {\n    margin-left: 0;\n}\n.cesium-measure-button:hover {\n    background-color: #48b;\n}\n.cesium-measure-button:last-child {\n    border-right: none;\n}\n\n.cesium-measure-button.active {\n    background-color: #1a71cc;\n    cursor: default;\n}\n.cesium-measure-button-main {\n    stroke-width: 1.3;\n}\n.cesium-measure-help {\n    stroke-width: 0;\n    padding: 6px 8px;\n    fill: #edffff;\n}\n.cesium-measure-instructions {\n    position: absolute;\n    max-height: 500px;\n    width: 403px;\n    border: 1px solid #444;\n    overflow-y: auto;\n    border-radius: 0;\n    background-color: rgba(48,51,54,0.8);\n    color: #edffff;\n    padding: 10px;\n    stroke: #edffff;\n    stroke-width: 2;\n}\n.cesium-measure-icon {\n    margin-right: 10px;\n}\n.cesium-measure-instructions .bold {\n    margin-bottom: 3px;\n}\n.cesium-measure-instructions > ul {\n    padding-left: 20px;\n}\n";
  styleInject(css_248z);

  var css_248z$1 = ".cesium-viewer-measureContainer {\r\n    position: relative;\r\n    display: inline-block;\r\n    margin: 0 3px;\r\n    vertical-align: middle;\r\n}";
  styleInject(css_248z$1);

  var DistanceUnits = {
      METERS: 'METERS',

      CENTIMETERS: 'CENTIMETERS',
      KILOMETERS: 'KILOMETERS',
      FEET: 'FEET',
      US_SURVEY_FEET: 'US_SURVEY_FEET',
      INCHES: 'INCHES',
      YARDS: 'YARDS',
      MILES: 'MILES'
  };

  var AreaUnits = {
      SQUARE_METERS: 'SQUARE_METERS',

      SQUARE_CENTIMETERS: 'SQUARE_CENTIMETERS',
      SQUARE_KILOMETERS: 'SQUARE_KILOMETERS',
      SQUARE_FEET: 'SQUARE_FEET',
      SQUARE_INCHES: 'SQUARE_INCHES',
      SQUARE_YARDS: 'SQUARE_YARDS',
      SQUARE_MILES: 'SQUARE_MILES',
      ACRES: 'ACRES',
      HECTARES: 'HECTARES'
  };

  var VolumeUnits = {
      CUBIC_METERS: 'CUBIC_METERS',
      CUBIC_CENTIMETERS: 'CUBIC_CENTIMETERS',
      CUBIC_KILOMETERS: 'CUBIC_KILOMETERS',
      CUBIC_FEET: 'CUBIC_FEET',
      CUBIC_INCHES: 'CUBIC_INCHES',
      CUBIC_YARDS: 'CUBIC_YARDS',
      CUBIC_MILES: 'CUBIC_MILES'
  };

  var AngleUnits = {
      DEGREES: 'DEGREES',
      RADIANS: 'RADIANS',
      DEGREES_MINUTES_SECONDS: 'DEGREES_MINUTES_SECONDS',
      GRADE: 'GRADE',
      RATIO: 'RATIO'
  };

  function MeasureUnits(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      this.distanceUnits = cesium.defaultValue(options.distanceUnits, DistanceUnits.METERS);
      this.areaUnits = cesium.defaultValue(options.areaUnits, AreaUnits.SQUARE_METERS);
      this.volumeUnits = cesium.defaultValue(options.volumeUnits, VolumeUnits.CUBIC_METERS);
      this.angleUnits = cesium.defaultValue(options.angleUnits, AngleUnits.DEGREES);
      this.slopeUnits = cesium.defaultValue(options.slopeUnits, AngleUnits.DEGREES);
  }

  MeasureUnits.convertDistance = function (distance, from, to) {
      if (from === to) {
          return distance;
      }
      var toMeters = getDistanceUnitConversion(from);
      var fromMeters = 1.0 / getDistanceUnitConversion(to);
      return distance * toMeters * fromMeters;
  };

  MeasureUnits.convertArea = function (area, from, to) {
      if (from === to) {
          return area;
      }
      var toMeters = getAreaUnitConversion(from);
      var fromMeters = 1.0 / getAreaUnitConversion(to);
      return area * toMeters * fromMeters;
  };

  MeasureUnits.convertVolume = function (volume, from, to) {
      if (from === to) {
          return volume;
      }
      var toMeters = getVolumeUnitConversion(from);
      var fromMeters = 1.0 / getVolumeUnitConversion(to);
      return volume * toMeters * fromMeters;
  };

  MeasureUnits.convertAngle = function (angle, from, to) {
      if (from === to) {
          return angle;
      }
      var radians = convertAngleToRadians(angle, from);
      return convertAngleFromRadians(radians, to);
  };

  MeasureUnits.numberToString = function (number, selectedLocale, maximumFractionDigits, minimumFractionDigits) {
      return numberToFormattedString(number, selectedLocale, maximumFractionDigits, minimumFractionDigits);
  };

  MeasureUnits.distanceToString = function (meters, distanceUnits, selectedLocale, maximumFractionDigits, minimumFractionDigits) {
      var distance = MeasureUnits.convertDistance(meters, DistanceUnits.METERS, distanceUnits);
      return numberToFormattedString(distance, selectedLocale, maximumFractionDigits, minimumFractionDigits) + MeasureUnits.getDistanceUnitSpacing(distanceUnits) + MeasureUnits.getDistanceUnitSymbol(distanceUnits);
  };

  MeasureUnits.areaToString = function (metersSquared, areaUnits, selectedLocale, maximumFractionDigits, minimumFractionDigits) {
      var area = MeasureUnits.convertArea(metersSquared, AreaUnits.SQUARE_METERS, areaUnits);
      return numberToFormattedString(area, selectedLocale, maximumFractionDigits, minimumFractionDigits) + MeasureUnits.getAreaUnitSpacing(areaUnits) + MeasureUnits.getAreaUnitSymbol(areaUnits);
  };

  MeasureUnits.volumeToString = function (metersCubed, volumeUnits, selectedLocale, maximumFractionDigits, minimumFractionDigits) {
      var volume = MeasureUnits.convertVolume(metersCubed, VolumeUnits.CUBIC_METERS, volumeUnits);
      return numberToFormattedString(volume, selectedLocale, maximumFractionDigits, minimumFractionDigits) + MeasureUnits.getVolumeUnitSpacing(volumeUnits) + MeasureUnits.getVolumeUnitSymbol(volumeUnits);
  };

  MeasureUnits.angleToString = function (angleRadians, angleUnits, selectedLocale, maximumFractionDigits, minimumFractionDigits) {
      if (angleUnits === AngleUnits.DEGREES || angleUnits === AngleUnits.RADIANS || angleUnits === AngleUnits.GRADE) {
          var angle = convertAngleFromRadians(angleRadians, angleUnits);
          return numberToFormattedString(angle, selectedLocale, maximumFractionDigits, minimumFractionDigits) + MeasureUnits.getAngleUnitSpacing(angleUnits) + MeasureUnits.getAngleUnitSymbol(angleUnits);
      } else if (angleUnits === AngleUnits.DEGREES_MINUTES_SECONDS) {
          var deg = cesium.Math.toDegrees(angleRadians);
          var sign = deg < 0 ? '-' : '';
          deg = Math.abs(deg);
          var d = Math.floor(deg);
          var minfloat = (deg - d) * 60;
          var m = Math.floor(minfloat);
          var s = (minfloat - m) * 60;
          s = numberToFormattedString(s, undefined, maximumFractionDigits, minimumFractionDigits); // The locale is undefined so that a period is used instead of a comma for the decimal
          return sign + d + '° ' + m + '\' ' + s + '"';
      } else if (angleUnits === AngleUnits.RATIO) {
          var riseOverRun = convertAngleFromRadians(angleRadians, angleUnits);
          var run = 1.0 / riseOverRun;
          return '1:' + numberToFormattedString(run, selectedLocale, maximumFractionDigits, 0);
      }
  };

  MeasureUnits.longitudeToString = function (longitude, angleUnits, selectedLocale, maximumFractionDigits, minimumFractionDigits) {
      return MeasureUnits.angleToString(Math.abs(longitude), angleUnits, selectedLocale, maximumFractionDigits, minimumFractionDigits) + ' ' + (longitude < 0.0 ? 'W' : 'E');
  };

  MeasureUnits.latitudeToString = function (latitude, angleUnits, selectedLocale, maximumFractionDigits, minimumFractionDigits) {
      return MeasureUnits.angleToString(Math.abs(latitude), angleUnits, selectedLocale, maximumFractionDigits, minimumFractionDigits) + ' ' + (latitude < 0.0 ? 'S' : 'N');
  };

  MeasureUnits.getDistanceUnitSymbol = function (distanceUnits) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.typeOf.string('distanceUnits', distanceUnits);
      //>>includeEnd('debug');

      if (distanceUnits === DistanceUnits.METERS) {
          return 'm';
      } else if (distanceUnits === DistanceUnits.CENTIMETERS) {
          return 'cm';
      } else if (distanceUnits === DistanceUnits.KILOMETERS) {
          return 'km';
      } else if (distanceUnits === DistanceUnits.FEET || distanceUnits === DistanceUnits.US_SURVEY_FEET) {
          return 'ft';
      } else if (distanceUnits === DistanceUnits.INCHES) {
          return 'in';
      } else if (distanceUnits === DistanceUnits.YARDS) {
          return 'yd';
      } else if (distanceUnits === DistanceUnits.MILES) {
          return 'mi';
      }
      //>>includeStart('debug', pragmas.debug);
      throw new cesium.DeveloperError('Invalid distance units: ' + distanceUnits);
      //>>includeEnd('debug');
  };

  MeasureUnits.getDistanceUnitSpacing = function (distanceUnits) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.typeOf.string('distanceUnits', distanceUnits);
      //>>includeEnd('debug');

      return ' ';
  };

  MeasureUnits.getAreaUnitSymbol = function (areaUnits) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.typeOf.string('areaUnits', areaUnits);
      //>>includeEnd('debug');

      if (areaUnits === AreaUnits.SQUARE_METERS) {
          return 'm²';
      } else if (areaUnits === AreaUnits.SQUARE_CENTIMETERS) {
          return 'cm²';
      } else if (areaUnits === AreaUnits.SQUARE_KILOMETERS) {
          return 'km²';
      } else if (areaUnits === AreaUnits.SQUARE_FEET) {
          return 'sq ft';
      } else if (areaUnits === AreaUnits.SQUARE_INCHES) {
          return 'sq in';
      } else if (areaUnits === AreaUnits.SQUARE_YARDS) {
          return 'sq yd';
      } else if (areaUnits === AreaUnits.SQUARE_MILES) {
          return 'sq mi';
      } else if (areaUnits === AreaUnits.ACRES) {
          return 'ac';
      } else if (areaUnits === AreaUnits.HECTARES) {
          return 'ha';
      }
      //>>includeStart('debug', pragmas.debug);
      throw new cesium.DeveloperError('Invalid area units: ' + areaUnits);
      //>>includeEnd('debug');
  };

  MeasureUnits.getAreaUnitSpacing = function (areaUnits) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.typeOf.string('areaUnits', areaUnits);
      //>>includeEnd('debug');

      return ' ';
  };

  MeasureUnits.getVolumeUnitSymbol = function (volumeUnits) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.typeOf.string('volumeUnits', volumeUnits);
      //>>includeEnd('debug');

      if (volumeUnits === VolumeUnits.CUBIC_METERS) {
          return 'm³';
      } else if (volumeUnits === VolumeUnits.CUBIC_CENTIMETERS) {
          return 'cm³';
      } else if (volumeUnits === VolumeUnits.CUBIC_KILOMETERS) {
          return 'km³';
      } else if (volumeUnits === VolumeUnits.CUBIC_FEET) {
          return 'cu ft';
      } else if (volumeUnits === VolumeUnits.CUBIC_INCHES) {
          return 'cu in';
      } else if (volumeUnits === VolumeUnits.CUBIC_YARDS) {
          return 'cu yd';
      } else if (volumeUnits === VolumeUnits.CUBIC_MILES) {
          return 'cu mi';
      }
      //>>includeStart('debug', pragmas.debug);
      throw new cesium.DeveloperError('Invalid volume units: ' + volumeUnits);
      //>>includeEnd('debug');
  };

  MeasureUnits.getVolumeUnitSpacing = function (volumeUnits) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.typeOf.string('volumeUnits', volumeUnits);
      //>>includeEnd('debug');

      return ' ';
  };

  MeasureUnits.getAngleUnitSymbol = function (angleUnits) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.typeOf.string('angleUnits', angleUnits);
      //>>includeEnd('debug');

      if (angleUnits === AngleUnits.DEGREES) {
          return '°';
      } else if (angleUnits === AngleUnits.RADIANS) {
          return 'rad';
      } else if (angleUnits === AngleUnits.GRADE) {
          return '%';
      }
      //>>includeStart('debug', pragmas.debug);
      throw new cesium.DeveloperError('Invalid angle units: ' + angleUnits);
      //>>includeEnd('debug');
  };

  MeasureUnits.getAngleUnitSpacing = function (angleUnits) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.typeOf.string('angleUnits', angleUnits);
      //>>includeEnd('debug');

      if (angleUnits === AngleUnits.RADIANS) {
          return ' ';
      }
      return '';
  };

  var negativeZero = -0.0;
  var positiveZero = 0.0;
  function numberToFormattedString(number, selectedLocale, maximumFractionDigits, minimumFractionDigits) {
      maximumFractionDigits = cesium.defaultValue(maximumFractionDigits, 2);
      minimumFractionDigits = cesium.defaultValue(minimumFractionDigits, maximumFractionDigits);
      var localeStringOptions = {
          minimumFractionDigits: minimumFractionDigits,
          maximumFractionDigits: maximumFractionDigits
      };
      // If locale is undefined, the runtime's default locale is used.
      var numberString = number.toLocaleString(selectedLocale, localeStringOptions);
      var negativeZeroString = negativeZero.toLocaleString(selectedLocale, localeStringOptions);
      if (numberString === negativeZeroString) {
          return positiveZero.toLocaleString(selectedLocale, localeStringOptions);
      }
      return numberString;
  }

  function getDistanceUnitConversion(distanceUnits) {
      if (distanceUnits === DistanceUnits.METERS) {
          return 1.0;
      } else if (distanceUnits === DistanceUnits.CENTIMETERS) {
          return 0.01;
      } else if (distanceUnits === DistanceUnits.KILOMETERS) {
          return 1000.0;
      } else if (distanceUnits === DistanceUnits.FEET) {
          return 0.3048;
      } else if (distanceUnits === DistanceUnits.US_SURVEY_FEET) {
          return 1200.0 / 3937.0;
      } else if (distanceUnits === DistanceUnits.INCHES) {
          return 0.0254;
      } else if (distanceUnits === DistanceUnits.YARDS) {
          return 0.9144;
      } else if (distanceUnits === DistanceUnits.MILES) {
          return 1609.344;
      }
      //>>includeStart('debug', pragmas.debug);
      throw new cesium.DeveloperError('Invalid distance units:' + distanceUnits);
      //>>includeEnd('debug');
  }

  function getAreaUnitConversion(areaUnits) {
      if (areaUnits === AreaUnits.SQUARE_METERS) {
          return 1.0;
      } else if (areaUnits === AreaUnits.SQUARE_CENTIMETERS) {
          return 0.0001;
      } else if (areaUnits === AreaUnits.SQUARE_KILOMETERS) {
          return 1000000.0;
      } else if (areaUnits === AreaUnits.SQUARE_FEET) {
          return 0.3048 * 0.3048;
      } else if (areaUnits === AreaUnits.SQUARE_INCHES) {
          return 0.0254 * 0.0254;
      } else if (areaUnits === AreaUnits.SQUARE_YARDS) {
          return 0.9144 * 0.9144;
      } else if (areaUnits === AreaUnits.SQUARE_MILES) {
          return 1609.344 * 1609.344;
      } else if (areaUnits === AreaUnits.ACRES) {
          return 4046.85642232;
      } else if (areaUnits === AreaUnits.HECTARES) {
          return 10000.0;
      }
      //>>includeStart('debug', pragmas.debug);
      throw new cesium.DeveloperError('Invalid area units:' + areaUnits);
      //>>includeEnd('debug');
  }

  function getVolumeUnitConversion(volumeUnits) {
      if (volumeUnits === VolumeUnits.CUBIC_METERS) {
          return 1.0;
      } else if (volumeUnits === VolumeUnits.CUBIC_CENTIMETERS) {
          return 0.000001;
      } else if (volumeUnits === VolumeUnits.CUBIC_KILOMETERS) {
          return 1000000000.0;
      } else if (volumeUnits === VolumeUnits.CUBIC_FEET) {
          return 0.3048 * 0.3048 * 0.3048;
      } else if (volumeUnits === VolumeUnits.CUBIC_INCHES) {
          return 0.0254 * 0.0254 * 0.0254;
      } else if (volumeUnits === VolumeUnits.CUBIC_YARDS) {
          return 0.9144 * 0.9144 * 0.9144;
      } else if (volumeUnits === VolumeUnits.CUBIC_MILES) {
          return 1609.344 * 1609.344 * 1609.344;
      }
      //>>includeStart('debug', pragmas.debug);
      throw new cesium.DeveloperError('Invalid volume units:' + volumeUnits);
      //>>includeEnd('debug');
  }

  var degreesMinutesSecondsRegex = /(-?)(\d+)\s*°\s*(\d+)\s*'\s*([\d.,]+)"\s*([WENS]?)/i;

  function convertAngleToRadians(value, angleUnits) {
      if (angleUnits === AngleUnits.RADIANS) {
          return value;
      } else if (angleUnits === AngleUnits.DEGREES) {
          return cesium.Math.toRadians(value);
      } else if (angleUnits === AngleUnits.GRADE) {
          if (value === Number.POSITIVE_INFINITY) {
              return cesium.Math.PI_OVER_TWO;
          }
          return Math.atan(value / 100.0);
      } else if (angleUnits === AngleUnits.RATIO) {
          // Converts to radians where value is rise/run
          return Math.atan(value);
      } else if (angleUnits === AngleUnits.DEGREES_MINUTES_SECONDS) {
          var matches = degreesMinutesSecondsRegex.exec(value);
          if (!cesium.defined(matches)) {
              throw new cesium.RuntimeError('Could not convert angle to radians: ' + value);
          }
          var sign = matches[1].length > 0 ? -1.0 : 1.0;
          var degrees = parseInt(matches[2]);
          var minutes = parseInt(matches[3]);
          var seconds = parseFloat(matches[4]);
          var cardinal = matches[5];

          if (cardinal.length === 1) {
              cardinal = cardinal.toUpperCase();
              if (cardinal === 'W' || cardinal === 'S') {
                  sign *= -1.0;
              }
          }

          var degreesDecimal = sign * (degrees + minutes / 60.0 + seconds / 3600.0);
          return cesium.Math.toRadians(degreesDecimal);
      }

      //>>includeStart('debug', pragmas.debug);
      throw new cesium.DeveloperError('Invalid angle units: ' + angleUnits);
      //>>includeEnd('debug');
  }

  function convertAngleFromRadians(value, angleUnits) {
      if (angleUnits === AngleUnits.RADIANS) {
          return value;
      } else if (angleUnits === AngleUnits.DEGREES) {
          return cesium.Math.toDegrees(value);
      } else if (angleUnits === AngleUnits.GRADE) {
          value = cesium.Math.clamp(value, 0.0, cesium.Math.PI_OVER_TWO);
          if (value === cesium.Math.PI_OVER_TWO) {
              return Number.POSITIVE_INFINITY;
          }
          return 100.0 * Math.tan(value);
      } else if (angleUnits === AngleUnits.RATIO) {
          var rise = Math.sin(value);
          var run = Math.cos(value);
          return rise / run;
      }
      //>>includeStart('debug', pragmas.debug);
      throw new cesium.DeveloperError('Invalid angle units: ' + angleUnits);
      //>>includeEnd('debug');
  }

  /**
   * If element is a string, look up the element in the DOM by ID.  Otherwise return element.
   *
   * @private
   *
   * @exception {DeveloperError} Element with id "id" does not exist in the document.
   */
  function getElement(element) {
      if (typeof element === 'string') {
          var foundElement = document.getElementById(element);

          //>>includeStart('debug', pragmas.debug);
          if (foundElement === null) {
              throw new cesium.DeveloperError('Element with id "' + element + '" does not exist in the document.');
          }
          //>>includeEnd('debug');

          element = foundElement;
      }
      return element;
  }

  function Measurement(options) {
    options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);

    //>>includeStart('debug', pragmas.debug);
    cesium.Check.defined('options.scene', options.scene);
    cesium.Check.defined('options.units', options.units);
    cesium.Check.defined('options.points', options.points);
    cesium.Check.defined('options.labels', options.labels);
    cesium.Check.defined('options.primitives', options.primitives);
    //>>includeEnd('debug');

    this._labelCollection = options.labels;
    this._pointCollection = options.points;
    this._primitives = options.primitives;
    this._selectedUnits = options.units;
    this._selectedLocale = options.locale;
    this._scene = options.scene;
  }

  Object.defineProperties(Measurement.prototype, {
    /**
     * Gets the icon.
     * @type {String}
     * @memberof AreaMeasurement.prototype
     * @readonly
     */
    icon: {
      value: ''
    },
    /**
     * Gets the thumbnail.
     * @type {String}
     * @memberof AreaMeasurement.prototype
     * @readonly
     */
    thumbnail: {
      value: ''
    },
    /**
     * Gets the type.
     * @type {String}
     * @memberof AreaMeasurement.prototype
     * @readonly
     */
    type: {
      value: ''
    },
    /**
     * Gets the instruction text.
     * @type {String[]}
     * @memberof AreaMeasurement.prototype
     * @readonly
     */
    instructions: {
      value: []
    },
    /**
     * Gets the id.
     * @type {String}
     * @memberof AreaMeasurement.prototype
     * @readonly
     */
    id: {
      value: ''
    }
  });

  /**
   * Handles double click events while performing a measurement.
   */
  Measurement.prototype.handleDoubleClick = function () {};

  /**
   * Handles click events while performing a measurement.
   * @param {Cartesian2} clickPosition The click position
   */
  Measurement.prototype.handleClick = function (clickPosition) {};

  /**
   * Handles mouse move events while performing a measurement.
   * @param {Cartesian2} mousePosition The mouse position
   */
  Measurement.prototype.handleMouseMove = function (mousePosition) {};

  /**
   * Handles left down mouse events while performing a measurement.
   * @param {Cartesian2} mousePosition The mouse position
   */
  Measurement.prototype.handleLeftDown = function (mousePosition) {};

  /**
   * Handles left up mouse events while performing a measurement.
   * @param {Cartesian2} mousePosition The mouse position
   */
  Measurement.prototype.handleLeftUp = function (mousePosition) {};

  /**
   * Resets the widget.
   */
  Measurement.prototype.reset = cesium.DeveloperError.throwInstantiationError;

  /**
   * @returns {Boolean} true if the object has been destroyed, false otherwise.
   */
  Measurement.prototype.isDestroyed = function () {
    return false;
  };

  /**
   * Destroys the widget.
   */
  Measurement.prototype.destroy = cesium.DeveloperError.throwInstantiationError;

  var DrawingMode = {
      BeforeDraw: 0,
      Drawing: 1,
      AfterDraw: 2
  };

  function PolygonPrimitive(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);

      this.show = cesium.defaultValue(options.show, true);
      var color = cesium.Color.clone(cesium.defaultValue(options.color, cesium.Color.WHITE));
      this._id = cesium.createGuid();
      this._color = color;
      this._depthFailColor = color;
      this._positions = [];

      this._boundingSphere = new cesium.BoundingSphere();
      this._primitive = undefined;
      this._update = true;
  }

  Object.defineProperties(PolygonPrimitive.prototype, {
      positions: {
          get: function () {
              return this._positions;
          },
          set: function (positions) {
              this._positions = positions;
              this._update = true;
          }
      },
      color: {
          get: function () {
              return this._color;
          }
      },
      boundingVolume: {
          get: function () {
              return this._boundingSphere;
          }
      }
  });

  PolygonPrimitive.prototype.update = function (frameState) {
      if (!this.show) {
          return;
      }

      var positions = this._positions;
      if (positions.length < 3) {
          this._primitive = this._primitive && this._primitive.destroy();
          return;
      }

      if (this._update) {
          this._update = false;

          this._primitive = this._primitive && this._primitive.destroy();
          var geometry = cesium.CoplanarPolygonGeometry.fromPositions({
              positions: positions,
              vertexFormat: cesium.PerInstanceColorAppearance.FLAT_VERTEX_FORMAT
          });
          this._primitive = new cesium.Primitive({
              geometryInstances: new cesium.GeometryInstance({
                  geometry: geometry,
                  attributes: {
                      color: cesium.ColorGeometryInstanceAttribute.fromColor(this._color),
                      depthFailColor: cesium.ColorGeometryInstanceAttribute.fromColor(this._depthFailColor)
                  },
                  id: this._id
              }),
              appearance: new cesium.PerInstanceColorAppearance({
                  flat: true,
                  closed: false,
                  translucent: this._color.alpha < 1.0
              }),
              depthFailAppearance: new cesium.PerInstanceColorAppearance({
                  flat: true,
                  closed: false,
                  translucent: this._depthFailColor.alpha < 1.0
              }),
              allowPicking: false,
              asynchronous: false
          });
          this._boundingSphere = cesium.BoundingSphere.fromPoints(positions, this._boundingSphere);
      }

      this._primitive.update(frameState);
  };

  PolygonPrimitive.prototype.isDestroyed = function () {
      return false;
  };

  PolygonPrimitive.prototype.destroy = function () {
      this._primitive = this._primitive && this._primitive.destroy();
      return cesium.destroyObject(this);
  };

  function PolylinePrimitive(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);

      this.show = cesium.defaultValue(options.show, true);

      this._ellipsoid = cesium.defaultValue(options.ellipsoid, cesium.Ellipsoid.WGS84);
      this._width = cesium.defaultValue(options.width, 3);
      this._color = cesium.Color.clone(cesium.defaultValue(options.color, cesium.Color.WHITE));
      this._id = cesium.createGuid();
      this._positions = cesium.defaultValue(options.positions, []);
      this._primitive = undefined;
      this._boundingSphere = new cesium.BoundingSphere();
      this._dashed = cesium.defaultValue(options.dashed, false);
      this._loop = cesium.defaultValue(options.loop, false);

      this._update = true;
  }

  Object.defineProperties(PolylinePrimitive.prototype, {
      positions: {
          get: function () {
              return this._positions;
          },
          set: function (positions) {
              this._positions = positions;
              this._update = true;
          }
      },
      color: {
          get: function () {
              return this._color;
          }
      },
      boundingVolume: {
          get: function () {
              return this._boundingSphere;
          }
      },
      width: {
          get: function () {
              return this._width;
          }
      },
      ellipsoid: {
          get: function () {
              return this._ellipsoid;
          }
      },
      dashed: {
          get: function () {
              return this._dashed;
          }
      },
      loop: {
          get: function () {
              return this._loop;
          }
      }
  });

  PolylinePrimitive.prototype.update = function (frameState) {
      if (!this.show) {
          return;
      }

      var positions = this._positions;
      if (!cesium.defined(positions) || positions.length < 2) {
          this._primitive = this._primitive && this._primitive.destroy();
          return;
      }

      if (this._update) {
          this._update = false;
          this._id = this.id;

          this._primitive = this._primitive && this._primitive.destroy();
          if (this._loop) {
              positions = positions.slice();
              positions.push(positions[0]);
          }
          var geometry = new cesium.PolylineGeometry({
              positions: positions,
              width: this.width,
              vertexFormat: cesium.PolylineMaterialAppearance.VERTEX_FORMAT,
              ellipsoid: this._ellipsoid,
              arcType: cesium.ArcType.NONE
          });

          var appearance1;
          if (this._dashed) {
              appearance1 = new cesium.PolylineMaterialAppearance({
                  material: cesium.Material.fromType(cesium.Material.PolylineDashType, {
                      color: this._color
                  })
              });
          } else {
              appearance1 = new cesium.PolylineColorAppearance();
          }

          this._primitive = new cesium.Primitive({
              geometryInstances: new cesium.GeometryInstance({
                  geometry: geometry,
                  attributes: {
                      color: cesium.ColorGeometryInstanceAttribute.fromColor(this._color),
                      depthFailColor: cesium.ColorGeometryInstanceAttribute.fromColor(this._color)
                  },
                  id: this.id
              }),
              appearance: appearance1,
              depthFailAppearance: appearance1,
              asynchronous: false,
              allowPicking: false
          });
          this._boundingSphere = cesium.BoundingSphere.fromPoints(positions, this._boundingSphere);
      }

      this._primitive.update(frameState);
  };

  PolylinePrimitive.prototype.isDestroyed = function () {
      return false;
  };

  PolylinePrimitive.prototype.destroy = function () {
      this._primitive = this._primitive && this._primitive.destroy();
      return cesium.destroyObject(this);
  };

  function VisibilityState() {
      this.states = new cesium.ManagedArray();
      this.count = 0;
  }

  VisibilityState.prototype.hidePrimitiveCollection = function (primitiveCollection) {
      var primitivesLength = primitiveCollection.length;
      for (var i = 0; i < primitivesLength; ++i) {
          var primitive = primitiveCollection.get(i);
          if (primitive instanceof cesium.PrimitiveCollection) {
              this.hidePrimitiveCollection(primitive);
              continue;
          }

          this.states.push(primitive.show);

          if (primitive instanceof cesium.Cesium3DTileset || primitive instanceof cesium.Model) {
              continue;
          }
          primitive.show = false;
      }
  };

  VisibilityState.prototype.restorePrimitiveCollection = function (primitiveCollection) {
      var primitivesLength = primitiveCollection.length;
      for (var i = 0; i < primitivesLength; ++i) {
          var primitive = primitiveCollection.get(i);
          if (primitive instanceof cesium.PrimitiveCollection) {
              this.restorePrimitiveCollection(primitive);
              continue;
          }

          primitive.show = this.states.get(this.count++);
      }
  };

  VisibilityState.prototype.hide = function (scene) {
      this.states.length = 0;

      this.hidePrimitiveCollection(scene.primitives);
      this.hidePrimitiveCollection(scene.groundPrimitives);
  };

  VisibilityState.prototype.restore = function (scene) {
      this.count = 0;

      this.restorePrimitiveCollection(scene.primitives);
      this.restorePrimitiveCollection(scene.groundPrimitives);
  };

  var cartesianScratch = new cesium.Cartesian3();
  var rayScratch = new cesium.Ray();
  var visibilityState = new VisibilityState();

  function getWorldPosition(scene, mousePosition, result) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('scene', scene);
      cesium.Check.defined('mousePosition', mousePosition);
      cesium.Check.defined('result', result);
      //>>includeEnd('debug');
      var position;
      if (scene.pickPositionSupported) {
          // Hide every primitive that isn't a tileset
          visibilityState.hide(scene);

          // Don't pick default 3x3, or scene.pick may allow a mousePosition that isn't on the tileset to pickPosition.
          var pickedObject = scene.pick(mousePosition, 1, 1);

          visibilityState.restore(scene);

          if (cesium.defined(pickedObject) && (pickedObject instanceof cesium.Cesium3DTileFeature || pickedObject.primitive instanceof cesium.Cesium3DTileset || pickedObject.primitive instanceof cesium.Model)) {
              // check to let us know if we should pick against the globe instead
              position = scene.pickPosition(mousePosition, cartesianScratch);

              if (cesium.defined(position)) {
                  return cesium.Cartesian3.clone(position, result);
              }
          }
      }

      if (!cesium.defined(scene.globe)) {
          return;
      }

      var ray = scene.camera.getPickRay(mousePosition, rayScratch);
      position = scene.globe.pick(ray, scene, cartesianScratch);

      if (cesium.defined(position)) {
          return cesium.Cartesian3.clone(position, result);
      }
  }

  var clickDistanceScratch = new cesium.Cartesian2();
  var cart3Scratch = new cesium.Cartesian3();

  var mouseDelta = 10;

  function PolygonDrawing(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('options.scene', options.scene);
      //>>includeEnd('debug');

      var scene = options.scene;
      var primitives = cesium.defaultValue(options.primitives, scene.primitives);
      var removePoints = false;
      var points = options.points;
      if (!cesium.defined(points)) {
          points = primitives.add(new cesium.PointPrimitiveCollection());
          removePoints = true;
      }

      this._polygon = primitives.add(new PolygonPrimitive(options.polygonOptions));
      this._polyline = primitives.add(new PolylinePrimitive(cesium.combine({
          loop: true
      }, options.polylineOptions)));
      this._pointOptions = options.pointOptions;
      this._pointCollection = points;
      this._removePoints = removePoints;
      this._scene = scene;
      this._primitives = primitives;
      this._positions = [];
      this._points = [];
      this._tempNextPos = new cesium.Cartesian3();
      this._mode = DrawingMode.BeforeDraw;
      this._lastClickPosition = new cesium.Cartesian2(Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY);
  }

  /**
   * Adds a point to the polygon.
   * @param {Cartesian3} position The position to add
   * @private
   */
  PolygonDrawing.prototype.addPoint = function (position) {
      var positions = this._positions;
      positions.push(position);
      this._polyline.positions = positions;
      this._polygon.positions = positions;
      var point = this._pointCollection.add(this._pointOptions);
      point.position = position;
      point.show = true;
      this._points.push(point);
  };

  /**
   * Ends drawing on double click.
   */
  PolygonDrawing.prototype.handleDoubleClick = function () {
      // expect point to be added by handleClick
      this._mode = DrawingMode.AfterDraw;

      // Sometimes a move event is fired between the ending
      // click and doubleClick events, so make sure the polyline
      // and polygon have the correct positions.
      var positions = this._positions;
      this._polyline.positions = positions;
      this._polygon.positions = positions;
  };

  /**
   * Handles click events while drawing a polygon.
   * @param {Cartesian2} clickPosition The click position
   */
  PolygonDrawing.prototype.handleClick = function (clickPosition) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('clickPosition', clickPosition);
      //>>includeEnd('debug');

      if (this._mode === DrawingMode.AfterDraw) {
          return;
      }

      // Don't handle if clickPos is too close to previous click.
      // This typically indicates a double click handler will be fired next,
      // we don't expect the user to wait and click this point again.
      var lastClickPos = this._lastClickPosition;
      var distance = cesium.Cartesian2.magnitude(cesium.Cartesian2.subtract(lastClickPos, clickPosition, clickDistanceScratch));
      if (distance < mouseDelta) {
          return;
      }

      var position = PolygonDrawing._getWorldPosition(this._scene, clickPosition, cart3Scratch);
      if (!cesium.defined(position)) {
          return;
      }

      this.addPoint(cesium.Cartesian3.clone(position, new cesium.Cartesian3()));
      this._mode = DrawingMode.Drawing;

      cesium.Cartesian2.clone(clickPosition, lastClickPos);

      return position;
  };

  /**
   * Handles mouse move events while drawing a polygon.
   * @param {Cartesian2} mousePosition The mouse position
   */
  PolygonDrawing.prototype.handleMouseMove = function (mousePosition) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('mousePosition', mousePosition);
      //>>includeEnd('debug');

      if (this._mode !== DrawingMode.Drawing) {
          return;
      }
      var scene = this._scene;
      var nextPos = PolygonDrawing._getWorldPosition(scene, mousePosition, cart3Scratch);
      if (!cesium.defined(nextPos)) {
          return;
      }
      var positions = this._positions.slice();
      positions.push(cesium.Cartesian3.clone(nextPos, this._tempNextPos));
      this._polyline.positions = positions;
      this._polygon.positions = positions;

      return nextPos;
  };

  /**
   * @returns {Boolean} true if the object has been destroyed, false otherwise.
   */
  PolygonDrawing.prototype.isDestroyed = function () {
      return false;
  };

  /**
   * Destroys the widget.
   */
  PolygonDrawing.prototype.destroy = function () {
      var primitives = this._primitives;
      if (this._removePoints) {
          primitives.remove(this._points);
      } else {
          var points = this._points;
          var pointCollection = this._pointCollection;
          for (var i = 0; i < points.length; i++) {
              pointCollection.remove(points[i]);
          }
      }

      primitives.remove(this._polygon);
      primitives.remove(this._polyline);

      return cesium.destroyObject(this);
  };

  // Exposed for specs
  PolygonDrawing._getWorldPosition = getWorldPosition;

  var defaultLabelPixelOffset = new cesium.Cartesian2(0, -9);

  var MeasurementSettings = {};

  MeasurementSettings.color = cesium.Color.YELLOW;

  MeasurementSettings.labelFont = '16px Lucida Console';

  MeasurementSettings.textColor = cesium.Color.WHITE;

  MeasurementSettings.backgroundColor = new cesium.Color(0.165, 0.165, 0.165, 0.8);

  MeasurementSettings.backgroundPadding = new cesium.Cartesian2(7, 5);

  MeasurementSettings.getPolylineOptions = function (options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      return {
          show: options.show,
          ellipsoid: options.ellipsoid,
          width: cesium.defaultValue(options.width, 3),
          color: cesium.defaultValue(options.color, MeasurementSettings.color),
          depthFailColor: cesium.defaultValue(cesium.defaultValue(options.depthFailColor, options.color), MeasurementSettings.color),
          id: options.id,
          positions: options.positions,
          materialType: options.materialType,
          depthFailMaterialType: options.depthFailMaterialType,
          loop: options.loop,
          clampToGround: options.clampToGround,
          classificationType: options.classificationType,
          allowPicking: cesium.defaultValue(options.allowPicking, false)
      };
  };

  MeasurementSettings.getPolygonOptions = function (options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      return {
          show: options.show,
          ellipsoid: options.ellipsoid,
          color: cesium.defaultValue(options.color, MeasurementSettings.color),
          depthFailColor: cesium.defaultValue(cesium.defaultValue(options.depthFailColor, options.color), MeasurementSettings.color),
          id: options.id,
          positions: options.positions,
          clampToGround: options.clampToGround,
          classificationType: options.classificationType,
          allowPicking: cesium.defaultValue(options.allowPicking, false)
      };
  };

  MeasurementSettings.getPointOptions = function () {
      return {
          pixelSize: 10,
          color: MeasurementSettings.color,
          position: new cesium.Cartesian3(),
          disableDepthTestDistance: Number.POSITIVE_INFINITY, // for draw-over
          show: false
      };
  };

  MeasurementSettings.getLabelOptions = function (options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      return {
          show: false,
          font: MeasurementSettings.labelFont,
          scale: cesium.defaultValue(options.scale, 1.0),
          fillColor: cesium.defaultValue(options.fillColor, MeasurementSettings.textColor),
          showBackground: true,
          backgroundColor: cesium.defaultValue(options.backgroundColor, MeasurementSettings.backgroundColor),
          backgroundPadding: cesium.defaultValue(options.backgroundPadding, MeasurementSettings.backgroundPadding),
          horizontalOrigin: cesium.defaultValue(options.horizontalOrigin, cesium.HorizontalOrigin.CENTER),
          verticalOrigin: cesium.defaultValue(options.verticalOrigin, cesium.VerticalOrigin.BOTTOM),
          pixelOffset: cesium.defined(options.pixelOffset) ? options.pixelOffset : cesium.Cartesian2.clone(defaultLabelPixelOffset),
          disableDepthTestDistance: Number.POSITIVE_INFINITY, // for draw-over
          position: new cesium.Cartesian3()
      };
  };

  var cart2Scratch1 = new cesium.Cartesian2();
  var cart2Scratch2 = new cesium.Cartesian2();

  var p0Scratch = new cesium.Cartesian3();
  var p1Scratch = new cesium.Cartesian3();
  var p2Scratch = new cesium.Cartesian3();
  var v0Scratch = new cesium.Cartesian3();
  var v1Scratch = new cesium.Cartesian3();

  function triangleArea(p0, p1, p2) {
      var v0 = cesium.Cartesian3.subtract(p0, p1, v0Scratch);
      var v1 = cesium.Cartesian3.subtract(p2, p1, v1Scratch);
      var cross = cesium.Cartesian3.cross(v0, v1, v0);
      return cesium.Cartesian3.magnitude(cross) * 0.5;
  }

  function AreaMeasurementDrawing(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('options.scene', options.scene);
      cesium.Check.defined('options.primitives', options.primitives);
      cesium.Check.defined('options.units', options.units);
      cesium.Check.defined('options.points', options.points);
      cesium.Check.defined('options.labels', options.labels);
      //>>includeEnd('debug');

      options.polylineOptions = MeasurementSettings.getPolylineOptions({
          ellipsoid: options.ellipsoid
      });
      options.pointOptions = MeasurementSettings.getPointOptions();
      options.polygonOptions = {
          color: MeasurementSettings.color.withAlpha(0.8)
      };
      PolygonDrawing.call(this, options);

      var labels = options.labels;
      this._labelCollection = labels;
      this._label = labels.add(MeasurementSettings.getLabelOptions());
      this._selectedUnits = options.units;
      this._selectedLocale = options.locale;
      this._area = 0;

      var that = this;
      this._removeEvent = this._scene.preRender.addEventListener(function () {
          that.updateLabel();
      });
  }

  AreaMeasurementDrawing.prototype = Object.create(PolygonDrawing.prototype);
  AreaMeasurementDrawing.prototype.constructor = AreaMeasurementDrawing;

  Object.defineProperties(AreaMeasurementDrawing.prototype, {
      /**
       * Gets the area value in meters squared
       * @type {Number}
       * @memberof AreaMeasurementDrawing.prototype
       * @readonly
       */
      area: {
          get: function () {
              return this._area;
          }
      }
  });

  /**
   * Computes the area of the polygon.
   * @param {Cartesian3[]} positions An array of positions
   * @private
   */
  AreaMeasurementDrawing.prototype.updateArea = function (positions) {
      var geometry = cesium.CoplanarPolygonGeometry.createGeometry(cesium.CoplanarPolygonGeometry.fromPositions({
          positions: positions,
          vertexFormat: cesium.VertexFormat.POSITION_ONLY
      }));
      if (!cesium.defined(geometry)) {
          this._label.text = '';
          return;
      }

      var flatPositions = geometry.attributes.position.values;
      var indices = geometry.indices;

      var area = 0;
      for (var i = 0; i < indices.length; i += 3) {
          var i0 = indices[i];
          var i1 = indices[i + 1];
          var i2 = indices[i + 2];

          var p0 = cesium.Cartesian3.unpack(flatPositions, i0 * 3, p0Scratch);
          var p1 = cesium.Cartesian3.unpack(flatPositions, i1 * 3, p1Scratch);
          var p2 = cesium.Cartesian3.unpack(flatPositions, i2 * 3, p2Scratch);
          area += triangleArea(p0, p1, p2);
      }

      this._area = area;
      this._label.text = MeasureUnits.areaToString(area, this._selectedUnits.areaUnits, this._selectedLocale);
  };

  /**
   * Updates the label position.
   * @private
   */
  AreaMeasurementDrawing.prototype.updateLabel = function () {
      var positions = this._positions;
      if (positions.length < 2) {
          return;
      }
      var scene = this._scene;
      var top = positions[0];
      var pos2d = cesium.SceneTransforms.wgs84ToWindowCoordinates(scene, top, cart2Scratch1);
      var lastY = cesium.defined(pos2d) ? pos2d.y : Number.POSITIVE_INFINITY;
      for (var i = 1; i < positions.length; i++) {
          var nextScreenPos = cesium.SceneTransforms.wgs84ToWindowCoordinates(scene, positions[i], cart2Scratch2);
          if (!cesium.defined(nextScreenPos)) {
              continue;
          }
          if (nextScreenPos.y < lastY) {
              lastY = nextScreenPos.y;
              top = positions[i];
          }
      }
      this._label.position = top;
  };

  /**
   * Adds a point to the polygon.
   * @param {Cartesian3} position The position to add
   * @private
   */
  AreaMeasurementDrawing.prototype.addPoint = function (position) {
      PolygonDrawing.prototype.addPoint.call(this, position);
      this.updateArea(this._positions);
  };

  /**
   * Handles click events while drawing a polygon.
   * @param {Cartesian2} clickPosition The click position
   */
  AreaMeasurementDrawing.prototype.handleClick = function (clickPosition) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('clickPosition', clickPosition);
      //>>includeEnd('debug');

      if (this._mode === DrawingMode.AfterDraw) {
          this.reset();
      }

      var position = PolygonDrawing.prototype.handleClick.call(this, clickPosition);
      if (cesium.defined(position)) {
          this._label.show = true;
          this._polygon.show = true;
          this._polyline.show = true;
      }
  };

  /**
   * Handles mouse move events while drawing a polygon.
   * @param {Cartesian2} mousePosition The mouse position
   */
  AreaMeasurementDrawing.prototype.handleMouseMove = function (mousePosition) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('mousePosition', mousePosition);
      //>>includeEnd('debug');

      var nextPos = PolygonDrawing.prototype.handleMouseMove.call(this, mousePosition);
      if (!cesium.defined(nextPos)) {
          return;
      }
      var positions = this._positions.slice();
      positions.push(nextPos);
      this.updateArea(positions);
  };

  /**
   * Resets the widget.
   */
  AreaMeasurementDrawing.prototype.reset = function () {
      this._label.show = false;
      this._label.text = '';
      this._positions = [];
      this._polyline.positions = [];
      this._polygon.positions = [];
      this._polyline.show = false;
      this._polygon.show = false;
      this._area = 0;
      var points = this._points;
      var pointCollection = this._pointCollection;
      for (var i = 0; i < points.length; i++) {
          pointCollection.remove(points[i]);
      }
      points.length = 0;

      this._mode = DrawingMode.BeforeDraw;
      this._lastClickPosition.x = Number.POSITIVE_INFINITY;
      this._lastClickPosition.y = Number.POSITIVE_INFINITY;
  };

  /**
   * Destroys the widget.
   */
  AreaMeasurementDrawing.prototype.destroy = function () {
      this._removeEvent();
      this._labelCollection.remove(this._label);

      PolygonDrawing.prototype.destroy.call(this);
  };

  function getIcon(size) {
      return '<svg viewBox="0 0 30 30" height="' + size + 'px" width="' + size + 'px">\n\
                 <g transform="translate(0,-267)">\n\
                   <circle r="2.0788691" cy="293.99896" cx="3.8532958"/>\n\
                   <circle r="2.0788691" cy="282.76791" cx="26.927404"/>\n\
                   <circle r="2.0788691" cy="270.20621" cx="4.0090437"/>\n\
                   <path d="m 26.326048,283.77014 -9.394396,5.02295 -9.3943948,5.02295 0.3471933,-10.64726 0.3471933,-10.64726 9.0472022,5.62431 z" transform="matrix(1.1625734,0,0,0.99297729,-4.6787891,1.2180486)"/>\n\
                 </g>\n\
               </svg>';
  }

  function AreaMeasurement(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      Measurement.call(this, options);

      this._drawing = new AreaMeasurementDrawing(options);
  }

  AreaMeasurement.prototype = Object.create(Measurement.prototype);
  AreaMeasurement.prototype.constructor = AreaMeasurement;

  Object.defineProperties(AreaMeasurement.prototype, {
      area: {
          get: function () {
              return this._drawing.area;
          }
      },

      icon: {
          value: getIcon(15)
      },

      thumbnail: {
          value: getIcon(25)
      },

      type: {
          value: 'Area'
      },

      instructions: {
          value: ['Click to start drawing a polygon', 'Keep clicking to add more points', 'Double click to finish drawing']
      },

      id: {
          value: 'areaMeasurement'
      }
  });

  AreaMeasurement.prototype.handleDoubleClick = function () {
      this._drawing.handleDoubleClick();
  };

  AreaMeasurement.prototype.handleClick = function (clickPosition) {
      this._drawing.handleClick(clickPosition);
  };

  AreaMeasurement.prototype.handleMouseMove = function (mousePosition) {
      this._drawing.handleMouseMove(mousePosition);
  };

  AreaMeasurement.prototype.reset = function () {
      this._drawing.reset();
  };

  AreaMeasurement.prototype.isDestroyed = function () {
      return false;
  };

  AreaMeasurement.prototype.destroy = function () {
      this._drawing.destroy();

      return cesium.destroyObject(this);
  };

  var Mode = {
      BeforeDraw: 0,
      Drawing: 1,
      AfterDraw: 2
  };

  var cart2Scratch1$1 = new cesium.Cartesian2();
  var cart2Scratch2$1 = new cesium.Cartesian2();
  var scratchCarto = new cesium.Cartographic();

  var cart3Scratch1 = new cesium.Cartesian3();
  var cart3Scratch2 = new cesium.Cartesian3();
  var cart3Scratch3 = new cesium.Cartesian3();

  function getIcon$1(size) {
      return '<svg viewBox="0 0 30 30" height="' + size + 'px" width="' + size + 'px">\n\
                 <g transform="translate(0,-267)">\n\
                  <path d="m 4.934989,292.6549 20.67981,-20.80395"/>\n\
                   <circle r="2.0788691" cy="270.1637" cx="27.025297"/>\n\
                   <circle r="2.0788691" cy="294.07068" cx="3.1183045"/>\n\
                 </g>\n\
               </svg>\n';
  }

  function getComponentIcon(size) {
      return '<svg viewBox="0 0 30 30" height="' + size + 'px" width="' + size + 'px">\n\
                 <g transform="translate(0,-267)">\n\
                   <path d="m 4.934989,292.6549 20.67981,-20.80395" />\n\
                   <circle r="2.0788691" cy="270.1637" cx="27.025297" />\n\
                   <circle r="2.0788691" cy="294.07068" cx="3.1183045" />\n\
                   <path style="stroke-dasharray:2.00314951, 1.00157475;stroke-dashoffset:0;" d="m 3.3194019,292.73274 -0.046996,-22.53109 21.6420984,-0.0266" />\n\
                 </g>\n\
               </svg>\n';
  }

  function DistanceMeasurement(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      Measurement.call(this, options);

      var that = this;
      var pointCollection = this._pointCollection;
      var labelCollection = this._labelCollection;
      var primitives = this._primitives;
      var scene = this._scene;

      var positions = [new cesium.Cartesian3(), new cesium.Cartesian3()];
      var xyPolylinePositions = [new cesium.Cartesian3(), new cesium.Cartesian3(), new cesium.Cartesian3()];
      var xyBoxPositions = [new cesium.Cartesian3(), new cesium.Cartesian3(), new cesium.Cartesian3()];

      var yPixelOffset = new cesium.Cartesian2(-9, 0);
      var xPixelOffset = new cesium.Cartesian2(9, 0);

      var ellipsoid = scene.frameState.mapProjection.ellipsoid;

      this._startPoint = pointCollection.add(MeasurementSettings.getPointOptions());
      this._endPoint = pointCollection.add(MeasurementSettings.getPointOptions());

      this._positions = positions;
      this._polyline = primitives.add(new PolylinePrimitive(MeasurementSettings.getPolylineOptions({
          ellipsoid: ellipsoid,
          width: 3,
          show: false,
          positions: positions
      })));

      this._xyPolylinePositions = xyPolylinePositions;
      this._xyPolyline = primitives.add(new PolylinePrimitive(MeasurementSettings.getPolylineOptions({
          ellipsoid: ellipsoid,
          width: 2,
          positions: xyPolylinePositions,
          materialType: cesium.Material.PolylineDashType
      })));

      this._xyBoxPositions = xyBoxPositions;
      this._xyBox = primitives.add(new PolylinePrimitive(MeasurementSettings.getPolylineOptions({
          ellipsoid: ellipsoid,
          width: 1,
          positions: xyBoxPositions
      })));

      this._label = labelCollection.add(MeasurementSettings.getLabelOptions({
          horizontalOrigin: cesium.HorizontalOrigin.LEFT,
          verticalOrigin: cesium.VerticalOrigin.TOP,
          pixelOffset: new cesium.Cartesian2(10, 10)
      }));

      this._xPixelOffset = xPixelOffset;
      this._xLabel = labelCollection.add(MeasurementSettings.getLabelOptions({
          scale: 0.6
      }));
      this._xAngleLabel = labelCollection.add(MeasurementSettings.getLabelOptions({
          scale: 0.6,
          horizontalOrigin: cesium.HorizontalOrigin.LEFT,
          verticalOrigin: cesium.VerticalOrigin.MIDDLE,
          pixelOffset: xPixelOffset
      }));

      this._yPixelOffset = yPixelOffset;
      this._yLabel = labelCollection.add(MeasurementSettings.getLabelOptions({
          scale: 0.6,
          horizontalOrigin: cesium.HorizontalOrigin.RIGHT,
          pixelOffset: yPixelOffset
      }));
      this._yAngleLabel = labelCollection.add(MeasurementSettings.getLabelOptions({
          scale: 0.6,
          verticalOrigin: cesium.VerticalOrigin.TOP,
          pixelOffset: new cesium.Cartesian2(0, 9)
      }));

      this._distance = 0;
      this._xDistance = 0;
      this._yDistance = 0;
      this._xAngle = 0;
      this._yAngle = 0;

      this._mode = Mode.BeforeDraw;
      this._showComponentLines = cesium.defaultValue(options.showComponentLines, false);

      this._removeEvent = scene.preRender.addEventListener(function () {
          that._updateLabelPosition();
      });
  }

  DistanceMeasurement.prototype = Object.create(Measurement.prototype);
  DistanceMeasurement.prototype.constructor = DistanceMeasurement;

  Object.defineProperties(DistanceMeasurement.prototype, {
      /**
       * Gets the distance of the measurement in meters
       * @type {Number}
       * @memberof DistanceMeasurement.prototype
       * @readonly
       */
      distance: {
          get: function () {
              return this._distance;
          }
      },
      /**
       * Gets the horizontal component of distance of the measurement in meters
       * @type {Number}
       * @memberof DistanceMeasurement.prototype
       * @readonly
       */
      horizontalDistance: {
          get: function () {
              return this._xDistance;
          }
      },
      /**
       * Gets the vertical component of the distance of the measurement in meters
       * @type {Number}
       * @memberof DistanceMeasurement.prototype
       * @readonly
       */
      verticalDistance: {
          get: function () {
              return this._yDistance;
          }
      },
      /**
       * Gets the angle between horizontal and the distance line in radians
       * @type {Number}
       * @memberof DistanceMeasurement.prototype
       * @readonly
       */
      angleFromHorizontal: {
          get: function () {
              return this._xAngle;
          }
      },
      /**
       * Gets the angle between vertical and the distance line in radians
       * @type {Number}
       * @memberof DistanceMeasurement.prototype
       * @readonly
       */
      angleFromVertical: {
          get: function () {
              return this._yAngle;
          }
      },
      /**
       * Gets the icon.
       * @type {String}
       * @memberof DistanceMeasurement.prototype
       * @readonly
       */
      icon: {
          get: function () {
              if (this._showComponentLines) {
                  return getComponentIcon(15);
              }
              return getIcon$1(15);
          }
      },
      /**
       * Gets the thumbnail.
       * @type {String}
       * @memberof DistanceMeasurement.prototype
       * @readonly
       */
      thumbnail: {
          get: function () {
              if (this._showComponentLines) {
                  return getComponentIcon(25);
              }
              return getIcon$1(25);
          }
      },
      /**
       * Gets the type.
       * @type {String}
       * @memberof DistanceMeasurement.prototype
       * @readonly
       */
      type: {
          get: function () {
              if (this._showComponentLines) {
                  return 'Component Distance';
              }
              return 'Distance';
          }
      },
      /**
       * Gets the instruction text.
       * @type {String[]}
       * @memberof DistanceMeasurement.prototype
       * @readonly
       */
      instructions: {
          value: ['Click on the point cloud or the globe to set the start point and end points', 'To make a new measurement, click to clear the previous measurement']
      },
      /**
       * Gets the id.
       * @type {String}
       * @memberof DistanceMeasurement.prototype
       * @readonly
       */
      id: {
          get: function () {
              if (this._showComponentLines) {
                  return 'componentDistanceMeasurement';
              }
              return 'distanceMeasurement';
          }
      },
      /**
       * Gets and sets whether or not to show the x and y component lines of the measurement.
       * @type {Boolean}
       * @memberof DistanceMeasurement.prototype
       * @default false
       */
      showComponentLines: {
          get: function () {
              return this._showComponentLines;
          },
          set: function (value) {
              this._showComponentLines = value;
              if (this._mode !== Mode.BeforeDraw) {
                  this._updateComponents();
              }
          }
      }
  });

  /**
   * Updates the label positions.
   * @private
   */
  DistanceMeasurement.prototype._updateComponents = function () {
      var show = this._showComponentLines;
      var xLabel = this._xLabel;
      var yLabel = this._yLabel;
      var xAngleLabel = this._xAngleLabel;
      var yAngleLabel = this._yAngleLabel;
      var xyPolyline = this._xyPolyline;
      var xyBox = this._xyBox;

      // always set to false first in case we can't compute the values.
      xLabel.show = false;
      yLabel.show = false;
      xAngleLabel.show = false;
      yAngleLabel.show = false;
      xyPolyline.show = false;
      xyBox.show = false;

      if (!show) {
          return;
      }

      var ellipsoid = this._scene.frameState.mapProjection.ellipsoid;

      var positions = this._positions;
      var p0 = positions[0];
      var p1 = positions[1];
      var height0 = ellipsoid.cartesianToCartographic(p0, scratchCarto).height;
      var height1 = ellipsoid.cartesianToCartographic(p1, scratchCarto).height;
      var bottomPoint;
      var topPoint;
      var topHeight;
      var bottomHeight;
      if (height0 < height1) {
          bottomPoint = p0;
          topPoint = p1;
          topHeight = height1;
          bottomHeight = height0;
      } else {
          bottomPoint = p1;
          topPoint = p0;
          topHeight = height0;
          bottomHeight = height1;
      }

      var xyPositions = this._xyPolylinePositions;
      xyPositions[0] = cesium.Cartesian3.clone(bottomPoint, xyPositions[0]);
      xyPositions[2] = cesium.Cartesian3.clone(topPoint, xyPositions[2]);
      var normal = ellipsoid.geodeticSurfaceNormal(bottomPoint, cart3Scratch1);
      normal = cesium.Cartesian3.multiplyByScalar(normal, topHeight - bottomHeight, normal);
      var corner = cesium.Cartesian3.add(bottomPoint, normal, xyPositions[1]);

      xyPolyline.positions = xyPositions;

      if (cesium.Cartesian3.equalsEpsilon(corner, topPoint, cesium.Math.EPSILON10) || cesium.Cartesian3.equalsEpsilon(corner, bottomPoint, cesium.Math.EPSILON10)) {
          return;
      }

      yLabel.show = true;
      xLabel.show = true;
      yAngleLabel.show = true;
      xAngleLabel.show = true;
      xyPolyline.show = true;
      xyBox.show = true;

      var v1 = cesium.Cartesian3.subtract(topPoint, corner, cart3Scratch1);
      var v2 = cesium.Cartesian3.subtract(bottomPoint, corner, cart3Scratch2);
      var mag = Math.min(cesium.Cartesian3.magnitude(v1), cesium.Cartesian3.magnitude(v2));
      var scale = mag > 15.0 ? mag * 0.15 : mag * 0.25;
      v1 = cesium.Cartesian3.normalize(v1, v1);
      v2 = cesium.Cartesian3.normalize(v2, v2);
      v1 = cesium.Cartesian3.multiplyByScalar(v1, scale, v1);
      v2 = cesium.Cartesian3.multiplyByScalar(v2, scale, v2);

      var boxPos = this._xyBoxPositions;
      boxPos[0] = cesium.Cartesian3.add(corner, v1, boxPos[0]);
      boxPos[1] = cesium.Cartesian3.add(boxPos[0], v2, boxPos[1]);
      boxPos[2] = cesium.Cartesian3.add(corner, v2, boxPos[2]);
      xyBox.positions = boxPos;

      xLabel.position = cesium.Cartesian3.midpoint(corner, topPoint, cart3Scratch1);
      yLabel.position = cesium.Cartesian3.midpoint(bottomPoint, corner, cart3Scratch1);
      xAngleLabel.position = cesium.Cartesian3.clone(topPoint, cart3Scratch1);
      yAngleLabel.position = cesium.Cartesian3.clone(bottomPoint, cart3Scratch1);

      var vx = cesium.Cartesian3.subtract(corner, topPoint, cart3Scratch2);
      var vy = cesium.Cartesian3.subtract(corner, bottomPoint, cart3Scratch1);
      var v = cesium.Cartesian3.subtract(topPoint, bottomPoint, cart3Scratch3);

      var yAngle = cesium.Cartesian3.angleBetween(vy, v);
      v = cesium.Cartesian3.negate(v, v);
      var xAngle = cesium.Cartesian3.angleBetween(vx, v);

      var xDistance = cesium.Cartesian3.magnitude(vx);
      var yDistance = cesium.Cartesian3.magnitude(vy);

      var selectedUnits = this._selectedUnits;
      var selectedLocale = this._selectedLocale;
      xLabel.text = MeasureUnits.distanceToString(xDistance, selectedUnits.distanceUnits, selectedLocale);
      yLabel.text = MeasureUnits.distanceToString(yDistance, selectedUnits.distanceUnits, selectedLocale);

      xAngleLabel.text = MeasureUnits.angleToString(xAngle, selectedUnits.slopeUnits, selectedLocale);
      yAngleLabel.text = MeasureUnits.angleToString(yAngle, selectedUnits.slopeUnits, selectedLocale);

      this._xDistance = xDistance;
      this._yDistance = yDistance;
      this._xAngle = xAngle;
      this._yAngle = yAngle;
  };

  /**
   * Updates the label positions.
   * @private
   */
  DistanceMeasurement.prototype._updateLabelPosition = function () {
      var positions = this._positions;
      if (this._mode === Mode.BeforeDraw) {
          return;
      }
      var scene = this._scene;
      var p0 = positions[0];
      var p1 = positions[1];

      var pos0 = cesium.SceneTransforms.wgs84ToWindowCoordinates(scene, p0, cart2Scratch1$1);
      var pos1 = cesium.SceneTransforms.wgs84ToWindowCoordinates(scene, p1, cart2Scratch2$1);

      if (!cesium.defined(pos0) || !cesium.defined(pos1)) {
          return;
      }

      var label = this._label;
      var yLabel = this._yLabel;
      var xAngleLabel = this._xAngleLabel;
      var m = (pos0.y - pos1.y) / (pos1.x - pos0.x);
      if (m > 0) {
          this._yPixelOffset.x = -9;
          this._xPixelOffset.x = 12;
          yLabel.pixelOffset = this._yPixelOffset;
          yLabel.horizontalOrigin = cesium.HorizontalOrigin.RIGHT;
          xAngleLabel.pixelOffset = this._xPixelOffset;
          xAngleLabel.horizontalOrigin = cesium.HorizontalOrigin.LEFT;
          label.horizontalOrigin = cesium.HorizontalOrigin.LEFT;
      } else {
          this._yPixelOffset.x = 9;
          this._xPixelOffset.x = -12;
          yLabel.pixelOffset = this._yPixelOffset;
          yLabel.horizontalOrigin = cesium.HorizontalOrigin.LEFT;
          xAngleLabel.pixelOffset = this._xPixelOffset;
          xAngleLabel.horizontalOrigin = cesium.HorizontalOrigin.RIGHT;
          label.horizontalOrigin = cesium.HorizontalOrigin.RIGHT;
      }
  };

  /**
   * Handles click events while drawing a distance measurement.
   * @param {Cartesian2} clickPosition The click position
   */
  DistanceMeasurement.prototype.handleClick = function (clickPosition) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('clickPosition', clickPosition);
      //>>includeEnd('debug');

      var scene = this._scene;
      if (this._mode === Mode.AfterDraw) {
          this.reset();
      }
      var mode = this._mode;

      var positions = this._positions;
      if (mode === Mode.BeforeDraw) {
          var pos = DistanceMeasurement._getWorldPosition(scene, clickPosition, positions[0]);
          if (!cesium.defined(pos)) {
              return;
          }
          this._polyline.show = true;
          positions[0] = pos.clone(positions[0]);
          positions[1] = pos.clone(positions[1]);
          this._startPoint.position = pos;
          this._startPoint.show = true;
          this._mode = Mode.Drawing;
          this._polyline.positions = positions;
      } else if (mode === Mode.Drawing) {
          this._endPoint.position = positions[1];
          this._endPoint.show = true;
          this._polyline.positions = positions;
          this._mode = Mode.AfterDraw;
      }
  };

  /**
   * Handles mouse move events while drawing a distance measurement.
   * @param {Cartesian2} mousePosition The mouse position
   */
  DistanceMeasurement.prototype.handleMouseMove = function (mousePosition) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('mousePosition', mousePosition);
      //>>includeEnd('debug');

      if (this._mode !== Mode.Drawing) {
          return;
      }

      var scene = this._scene;
      var positions = this._positions;
      var pos = DistanceMeasurement._getWorldPosition(scene, mousePosition, cart3Scratch1);

      if (!cesium.defined(pos)) {
          return;
      }

      var pos0 = positions[0];
      var pos1 = cesium.Cartesian3.clone(pos, positions[1]);

      var vec = cesium.Cartesian3.subtract(pos1, pos0, cart3Scratch1);
      var distance = cesium.Cartesian3.magnitude(vec);

      var label = this._label;
      label.position = cesium.Cartesian3.midpoint(pos0, pos1, cart3Scratch1);
      label.text = MeasureUnits.distanceToString(distance, this._selectedUnits.distanceUnits, this._selectedLocale);
      label.show = true;

      this._distance = distance;
      this._polyline.positions = positions;

      this._updateComponents();
  };

  /**
   * Resets the measurement.
   */
  DistanceMeasurement.prototype.reset = function () {
      this._polyline.show = false;
      this._xyPolyline.show = false;
      this._xyBox.show = false;
      this._label.show = false;
      this._xLabel.show = false;
      this._yLabel.show = false;
      this._xAngleLabel.show = false;
      this._yAngleLabel.show = false;
      this._startPoint.show = false;
      this._endPoint.show = false;
      this._mode = Mode.BeforeDraw;
      this._distance = 0;
      this._xDistance = 0;
      this._yDistance = 0;
      this._xAngle = 0;
      this._yAngle = 0;
  };

  /**
   * @returns {Boolean} true if the object has been destroyed, false otherwise.
   */
  DistanceMeasurement.prototype.isDestroyed = function () {
      return false;
  };

  /**
   * Destroys the measurement.
   */
  DistanceMeasurement.prototype.destroy = function () {
      this._removeEvent();

      var primitives = this._primitives;
      primitives.remove(this._polyline);
      primitives.remove(this._xyPolyline);
      primitives.remove(this._xyBox);

      var points = this._pointCollection;
      points.remove(this._startPoint);
      points.remove(this._endPoint);

      var labels = this._labelCollection;
      labels.remove(this._label);
      labels.remove(this._xLabel);
      labels.remove(this._yLabel);
      labels.remove(this._xAngleLabel);
      labels.remove(this._yAngleLabel);

      return cesium.destroyObject(this);
  };

  // Exposed for specs
  DistanceMeasurement._getWorldPosition = getWorldPosition;

  var scratch = new cesium.Cartesian3();
  var scratchCarto$1 = new cesium.Cartographic();

  function getIcon$2(size) {
      return '<svg viewBox="0 0 30 30" height="' + size + 'px" width="' + size + 'px">\n\
             <g transform="translate(0,-267)">\n\
               <path d="m 15.042838,272.34414 0.01712,19.60575"/>\n\
               <circle r="2.0788691" cy="270.01154" cx="15.078616"/>\n\
               <path d="m 0.64901081,296.20687 8.80039389,-6.01044 7.9375003,3.1183 12.347278,-3.34365"/>\n\
             </g>\n\
           </svg>';
  }

  function HeightMeasurement(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      Measurement.call(this, options);

      var positions = [new cesium.Cartesian3(), new cesium.Cartesian3()];
      var pointCollection = this._pointCollection;

      this._startPoint = pointCollection.add(MeasurementSettings.getPointOptions());
      this._endPoint = pointCollection.add(MeasurementSettings.getPointOptions());

      this._polyline = this._primitives.add(new PolylinePrimitive(MeasurementSettings.getPolylineOptions({
          ellipsoid: this._scene.frameState.mapProjection.ellipsoid,
          positions: positions
      })));

      this._label = this._labelCollection.add(MeasurementSettings.getLabelOptions({
          horizontalOrigin: cesium.HorizontalOrigin.LEFT,
          verticalOrigin: cesium.VerticalOrigin.TOP,
          pixelOffset: new cesium.Cartesian2(10, 10)
      }));

      this._positions = positions;
      this._distance = 0;
  }

  HeightMeasurement.prototype = Object.create(Measurement.prototype);
  HeightMeasurement.prototype.constructor = HeightMeasurement;

  Object.defineProperties(HeightMeasurement.prototype, {
      distance: {
          get: function () {
              return this._distance;
          }
      },
      icon: {
          value: getIcon$2(15)
      },
      thumbnail: {
          value: getIcon$2(25)
      },
      type: {
          value: 'Height from terrain'
      },

      instructions: {
          value: ['Click on the point cloud to get a distance from that point to terrain']
      },
      id: {
          value: 'heightMeasurement'
      }
  });

  HeightMeasurement.prototype.handleClick = function (clickPosition) {
      var scene = this._scene;
      this.reset();

      var positions = this._positions;

      var pos0 = HeightMeasurement._getWorldPosition(scene, clickPosition, positions[0]);
      if (!cesium.defined(pos0)) {
          return;
      }

      var globe = scene.globe;
      var ellipsoid = scene.frameState.mapProjection.ellipsoid;

      var carto = ellipsoid.cartesianToCartographic(pos0, scratchCarto$1);
      if (cesium.defined(globe)) {
          carto.height = cesium.defaultValue(globe.getHeight(carto), 0);
      } else {
          carto.height = 0;
      }
      var pos1 = ellipsoid.cartographicToCartesian(carto, positions[1]);

      var vec = cesium.Cartesian3.subtract(pos1, pos0, scratch);
      var distance = cesium.Cartesian3.magnitude(vec);

      var label = this._label;
      label.position = pos0;
      label.show = true;
      label.text = MeasureUnits.distanceToString(distance, this._selectedUnits.distanceUnits, this._selectedLocale);

      this._polyline.positions = positions;
      this._polyline.show = true;
      this._startPoint.position = pos0;
      this._startPoint.show = true;
      this._endPoint.position = pos1;
      this._endPoint.show = true;

      this._distance = distance;
  };

  HeightMeasurement.prototype.reset = function () {
      this._polyline.show = false;
      this._label.show = false;
      this._startPoint.show = false;
      this._endPoint.show = false;
      this._distance = 0;
  };

  HeightMeasurement.prototype.isDestroyed = function () {
      return false;
  };

  HeightMeasurement.prototype.destroy = function () {
      this._primitives.remove(this._polyline);

      var points = this._pointCollection;
      points.remove(this._startPoint);
      points.remove(this._endPoint);

      this._labelCollection.remove(this._label);

      return cesium.destroyObject(this);
  };

  // exposed for specs
  HeightMeasurement._getWorldPosition = getWorldPosition;

  var clickDistanceScratch$1 = new cesium.Cartesian2();
  var cart3Scratch$1 = new cesium.Cartesian3();

  var mouseDelta$1 = 10;

  function PolylineDrawing(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('options.scene', options.scene);
      //>>includeEnd('debug');

      var scene = options.scene;
      var primitives = cesium.defaultValue(options.primitives, scene.primitives);
      var points = options.points;
      var removePoints = false;
      if (!cesium.defined(points)) {
          points = primitives.add(new cesium.PointPrimitiveCollection());
          removePoints = true;
      }

      this._scene = scene;
      this._pointCollection = points;
      this._removePoints = removePoints;
      this._polyline = primitives.add(new PolylinePrimitive(options.polylineOptions));
      this._primitives = primitives;
      this._pointOptions = options.pointOptions;
      this._positions = [];
      this._points = [];
      this._tempNextPos = new cesium.Cartesian3();
      this._mode = DrawingMode.BeforeDraw;
      this._lastClickPosition = new cesium.Cartesian2(Number.POSITIVE_INFINITY, Number.POSITIVE_INFINITY);
  }

  /**
   * Adds a point to the polyline.
   * @param {Cartesian3} position The position to add
   * @private
   */
  PolylineDrawing.prototype.addPoint = function (position) {
      var positions = this._positions;
      positions.push(position);
      this._polyline.positions = positions;
      var point = this._pointCollection.add(this._pointOptions);
      point.position = position;
      point.show = true;
      this._points.push(point);
  };

  /**
   * Ends drawing on double click.
   */
  PolylineDrawing.prototype.handleDoubleClick = function () {
      // expect point to be added by handleClick
      this._mode = DrawingMode.AfterDraw;

      // Sometimes a move event is fired between the ending
      // click and doubleClick events, so make sure the polyline
      // has the correct positions.
      this._polyline.positions = this._positions;
  };

  /**
   * Handles click events while drawing a polyline.
   * @param {Cartesian2} clickPosition The click position
   */
  PolylineDrawing.prototype.handleClick = function (clickPosition) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('clickPosition', clickPosition);
      //>>includeEnd('debug');

      if (this._mode === DrawingMode.AfterDraw) {
          return;
      }

      // Don't handle if clickPos is too close to previous click.
      // This typically indicates a double click handler will be fired next,
      // we don't expect the user to wait and click this point again.
      var lastClickPos = this._lastClickPosition;
      var distance = cesium.Cartesian2.magnitude(cesium.Cartesian2.subtract(lastClickPos, clickPosition, clickDistanceScratch$1));
      if (distance < mouseDelta$1) {
          return;
      }

      var position = PolylineDrawing._getWorldPosition(this._scene, clickPosition, cart3Scratch$1);
      if (!cesium.defined(position)) {
          return;
      }

      this.addPoint(cesium.Cartesian3.clone(position, new cesium.Cartesian3()));
      this._mode = DrawingMode.Drawing;
      cesium.Cartesian2.clone(clickPosition, lastClickPos);
      return position;
  };

  /**
   * Handles mouse move events while drawing a polyline.
   * @param {Cartesian2} mousePosition The mouse position
   */
  PolylineDrawing.prototype.handleMouseMove = function (mousePosition) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('mousePosition', mousePosition);
      //>>includeEnd('debug');

      if (this._mode !== DrawingMode.Drawing) {
          return;
      }
      var scene = this._scene;
      var nextPos = PolylineDrawing._getWorldPosition(scene, mousePosition, cart3Scratch$1);
      if (!cesium.defined(nextPos)) {
          return;
      }
      var positions = this._positions.slice();
      positions.push(cesium.Cartesian3.clone(nextPos, this._tempNextPos));
      this._polyline.positions = positions;
      return nextPos;
  };

  /**
   * @returns {Boolean} true if the object has been destroyed, false otherwise.
   */
  PolylineDrawing.prototype.isDestroyed = function () {
      return false;
  };

  /**
   * Destroys the widget.
   */
  PolylineDrawing.prototype.destroy = function () {
      if (this._removePoints) {
          this._primitives.remove(this._points);
      } else {
          var points = this._points;
          var pointCollection = this._pointCollection;
          for (var i = 0; i < points.length; i++) {
              pointCollection.remove(points[i]);
          }
      }
      this._primitives.remove(this._polyline);

      return cesium.destroyObject(this);
  };

  // Exposed for specs
  PolylineDrawing._getWorldPosition = getWorldPosition;

  var clickDistanceScratch$2 = new cesium.Cartesian2();
  var cart3Scratch$2 = new cesium.Cartesian3();
  var cart3Scratch1$1 = new cesium.Cartesian3();
  var normalScratch = new cesium.Cartesian3();
  var rayScratch$1 = new cesium.Ray();
  var v1Scratch$1 = new cesium.Cartesian3();
  var v2Scratch = new cesium.Cartesian3();
  var cartoScratch = new cesium.Cartographic();

  var mouseDelta$2 = 10;

  function HorizontalPolylineDrawing(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('options.scene', options.scene);
      //>>includeEnd('debug');
      PolylineDrawing.call(this, options);
      var polylineOptions = cesium.defaultValue(options.polylineOptions, cesium.defaultValue.EMPTY_OBJECT);

      var dashLineOptions = {
          color: polylineOptions.color,
          ellipsoid: polylineOptions.ellipsoid,
          width: 2,
          dashed: true
      };
      var moveDashLine = this._primitives.add(new PolylinePrimitive(dashLineOptions));
      moveDashLine.positions = [new cesium.Cartesian3(), new cesium.Cartesian3()];
      moveDashLine.show = false;
      this._dashLineOptions = dashLineOptions;
      this._dashedLines = [];
      this._moveDashLine = moveDashLine;

      this._heightPlane = new cesium.Plane(cesium.Cartesian3.UNIT_X, 0);
      this._heightPlaneCV = new cesium.Plane(cesium.Cartesian3.UNIT_X, 0);
      this._firstMove = false;
      this._height = 0;
  }

  HorizontalPolylineDrawing.prototype = Object.create(PolylineDrawing.prototype);
  HorizontalPolylineDrawing.prototype.constructor = HorizontalPolylineDrawing;

  HorizontalPolylineDrawing.prototype._setDashLinePositions = function (line, position) {
      var globe = this._scene.globe;
      var ellipsoid = this._scene.frameState.mapProjection.ellipsoid;

      var positions = line.positions;
      positions[0] = cesium.Cartesian3.clone(position, positions[0]);

      var carto = ellipsoid.cartesianToCartographic(position, cartoScratch);
      if (cesium.defined(globe)) {
          carto.height = cesium.defaultValue(globe.getHeight(carto), 0);
      } else {
          carto.height = 0;
      }
      positions[1] = ellipsoid.cartographicToCartesian(carto, positions[1]);
      line.positions = positions;
  };

  /**
   * Adds a point to the polyline.
   * @param {Cartesian3} position The position to add
   * @private
   */
  HorizontalPolylineDrawing.prototype.addPoint = function (position) {
      PolylineDrawing.prototype.addPoint.call(this, position);

      var dashLine = this._primitives.add(new PolylinePrimitive(this._dashLineOptions));
      dashLine.positions = [new cesium.Cartesian3(), new cesium.Cartesian3()];
      this._dashedLines.push(dashLine);

      this._setDashLinePositions(dashLine, position);
  };

  /**
   * Ends drawing on double click.
   */
  HorizontalPolylineDrawing.prototype.handleDoubleClick = function () {
      // expect point to be added by handleClick
      this._mode = DrawingMode.AfterDraw;

      // Sometimes a move event is fired between the ending
      // click and doubleClick events, so make sure the polyline
      // has the correct positions.
      this._polyline.positions = this._positions;
      this._moveDashLine.show = false;
  };

  /**
   * Handles click events while drawing a polyline.
   * @param {Cartesian2} clickPosition The click position
   */
  HorizontalPolylineDrawing.prototype.handleClick = function (clickPosition) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('clickPosition', clickPosition);
      //>>includeEnd('debug');

      var pos;
      if (this._positions.length === 0) {
          var scene = this._scene;
          var ellipsoid = scene.frameState.mapProjection.ellipsoid;
          pos = PolylineDrawing.prototype.handleClick.call(this, clickPosition);
          if (!cesium.defined(pos)) {
              return;
          }
          this._heightPlane = cesium.Plane.fromPointNormal(pos, ellipsoid.geodeticSurfaceNormal(pos, normalScratch), this._heightPlane);

          var cartoPos = ellipsoid.cartesianToCartographic(pos, cartoScratch);
          var planePoint = scene.mapProjection.project(cartoPos, cart3Scratch1$1);
          var posCV = cesium.Cartesian3.fromElements(planePoint.z, planePoint.x, planePoint.y, planePoint);

          this._heightPlaneCV = cesium.Plane.fromPointNormal(posCV, cesium.Cartesian3.UNIT_X, this._heightPlaneCV);
          this._height = ellipsoid.cartesianToCartographic(pos, cartoScratch).height;
          this._firstMove = true;
      } else {
          // Don't handle if clickPos is too close to previous click.
          // This typically indicates a double click handler will be fired next,
          // we don't expect the user to wait and click this point again.
          var lastClickPos = this._lastClickPosition;
          var distance = cesium.Cartesian2.magnitude(cesium.Cartesian2.subtract(lastClickPos, clickPosition, clickDistanceScratch$2));
          if (distance < mouseDelta$2) {
              return;
          }
          cesium.Cartesian2.clone(clickPosition, lastClickPos);
          pos = cesium.Cartesian3.clone(this._tempNextPos);
          this.addPoint(pos);
          this._firstMove = true;
      }
      return pos;
  };

  /**
   * Handles mouse move events while drawing a polyline.
   * @param {Cartesian2} mousePosition The mouse position
   * @param {Boolean} shift True if the shift key was pressed
   */
  HorizontalPolylineDrawing.prototype.handleMouseMove = function (mousePosition, shift) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('mousePosition', mousePosition);
      cesium.Check.defined('shift', shift);
      //>>includeEnd('debug');

      if (this._mode !== DrawingMode.Drawing) {
          return;
      }
      var scene = this._scene;
      var ellipsoid = scene.frameState.mapProjection.ellipsoid;
      var positions = this._positions;

      var nextPos;
      var ray = scene.camera.getPickRay(mousePosition, rayScratch$1);
      if (scene.mode === cesium.SceneMode.SCENE3D) {
          nextPos = cesium.IntersectionTests.rayPlane(ray, this._heightPlane, cart3Scratch$2);
      } else if (scene.mode === cesium.SceneMode.COLUMBUS_VIEW) {
          nextPos = cesium.IntersectionTests.rayPlane(ray, this._heightPlaneCV, cart3Scratch$2);
          nextPos = cesium.Cartesian3.fromElements(nextPos.y, nextPos.z, nextPos.x, nextPos);
          var carto = scene.mapProjection.unproject(nextPos, cartoScratch);
          nextPos = ellipsoid.cartographicToCartesian(carto, nextPos);
      } else {
          nextPos = scene.camera.pickEllipsoid(mousePosition, ellipsoid, cart3Scratch$2);
          if (cesium.defined(nextPos)) {
              var cartoPos = ellipsoid.cartesianToCartographic(nextPos, cartoScratch);
              cartoPos.height = this._height;
              nextPos = ellipsoid.cartographicToCartesian(cartoPos, nextPos);
          }
      }

      if (!cesium.defined(nextPos)) {
          return;
      }

      if (!this._firstMove && shift) {
          var anchorPos = positions[positions.length - 1];
          var lastPos = this._tempNextPos;
          var direction = cesium.Cartesian3.subtract(lastPos, anchorPos, v1Scratch$1);
          var newDirection = cesium.Cartesian3.subtract(nextPos, anchorPos, v2Scratch);
          newDirection = cesium.Cartesian3.projectVector(newDirection, direction, newDirection);
          nextPos = cesium.Cartesian3.add(anchorPos, newDirection, nextPos);
      }

      positions = positions.slice();
      positions.push(cesium.Cartesian3.clone(nextPos, this._tempNextPos));
      this._polyline.positions = positions;
      this._firstMove = false;
      this._moveDashLine.show = true;
      this._setDashLinePositions(this._moveDashLine, nextPos);

      return nextPos;
  };

  /**
   * @returns {Boolean} true if the object has been destroyed, false otherwise.
   */
  HorizontalPolylineDrawing.prototype.isDestroyed = function () {
      return false;
  };

  /**
   * Destroys the widget.
   */
  HorizontalPolylineDrawing.prototype.destroy = function () {
      var primitives = this._primitives;
      var dashLines = this._dashedLines;
      for (var i = 0; i < dashLines.length; i++) {
          primitives.remove(dashLines[i]);
      }
      primitives.remove(this._moveDashLine);

      PolylineDrawing.prototype.destroy.call(this);
  };

  var cart3Scratch$3 = new cesium.Cartesian3();
  var cart2Scratch1$2 = new cesium.Cartesian2();
  var cart2Scratch2$2 = new cesium.Cartesian2();
  var cart2Scratch3 = new cesium.Cartesian2();
  var v1Scratch$2 = new cesium.Cartesian3();

  function HorizontalMeasurementDrawing(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('options.scene', options.scene);
      cesium.Check.defined('options.primitives', options.primitives);
      cesium.Check.defined('options.units', options.units);
      cesium.Check.defined('options.points', options.points);
      cesium.Check.defined('options.labels', options.labels);
      //>>includeEnd('debug');
      HorizontalPolylineDrawing.call(this, options);

      var labels = options.labels;
      this._labelCollection = labels;
      this._label = labels.add(MeasurementSettings.getLabelOptions({
          horizontalOrigin: cesium.HorizontalOrigin.LEFT,
          verticalOrigin: cesium.VerticalOrigin.BOTTOM,
          pixelOffset: new cesium.Cartesian2(10, -10)
      }));
      this._segmentLabels = [];
      this._selectedUnits = options.units;
      this._selectedLocale = options.locale;
      this._previousDistance = 0;
      this._distance = 0;

      var that = this;
      this._removeEvent = this._scene.preRender.addEventListener(function () {
          that.updateLabels();
      });
  }

  HorizontalMeasurementDrawing.prototype = Object.create(HorizontalPolylineDrawing.prototype);
  HorizontalMeasurementDrawing.prototype.constructor = HorizontalMeasurementDrawing;

  Object.defineProperties(HorizontalMeasurementDrawing.prototype, {
      /**
       * Gets the distance in meters
       * @type {Number}
       * @memberof HorizontalMeasurementDrawing.prototype
       * @readonly
       */
      distance: {
          get: function () {
              return this._distance;
          }
      }
  });

  /**
   * Updates the label position.
   * @private
   */
  HorizontalMeasurementDrawing.prototype.updateLabels = function () {
      var positions = this._positions;
      if (positions.length < 2) {
          return;
      }
      var scene = this._scene;
      var top = positions[0];
      var pos2d = cesium.SceneTransforms.wgs84ToWindowCoordinates(scene, top, cart2Scratch1$2);
      var lastScreenPos = cesium.defined(pos2d) ? cesium.Cartesian2.clone(pos2d, cart2Scratch3) : cesium.Cartesian2.fromElements(Number.NEGATIVE_INFINITY, Number.POSITIVE_INFINITY, cart2Scratch3);
      var topY = lastScreenPos.y;
      var labels = this._segmentLabels;
      labels[0].show = this._polyline.positions.length > 2;
      for (var i = 1; i < positions.length; i++) {
          var nextScreenPos = cesium.SceneTransforms.wgs84ToWindowCoordinates(scene, positions[i], cart2Scratch2$2);
          if (!cesium.defined(nextScreenPos)) {
              continue;
          }

          var m = (lastScreenPos.y - nextScreenPos.y) / (nextScreenPos.x - lastScreenPos.x);
          var label = labels[i - 1];
          if (m > 0) {
              label.horizontalOrigin = cesium.HorizontalOrigin.LEFT;
          } else {
              label.horizontalOrigin = cesium.HorizontalOrigin.RIGHT;
          }

          if (nextScreenPos.y < topY) {
              topY = nextScreenPos.y;
              top = positions[i];
          }
          lastScreenPos = cesium.Cartesian2.clone(nextScreenPos, lastScreenPos);
      }
      if (this._mode === DrawingMode.AfterDraw) {
          this._label.position = top;
      }
  };

  HorizontalMeasurementDrawing.prototype.addPoint = function (position) {
      var positions = this._positions;
      if (positions.length > 0) {
          // store distance that was calculated on mouse move
          this._previousDistance = this._distance;

          var label = this._labelCollection.add(MeasurementSettings.getLabelOptions({
              scale: 0.8,
              horizontalOrigin: cesium.HorizontalOrigin.LEFT,
              verticalOrigin: cesium.VerticalOrigin.TOP,
              pixelOffset: new cesium.Cartesian2(5, 5)
          }));
          var p1 = positions[positions.length - 1];
          label.position = cesium.Cartesian3.midpoint(p1, position, new cesium.Cartesian3());
          label.text = MeasureUnits.distanceToString(cesium.Cartesian3.distance(p1, position), this._selectedUnits.distanceUnits, this._selectedLocale);
          label.show = true;
          this._segmentLabels.push(label);
      }
      HorizontalPolylineDrawing.prototype.addPoint.call(this, position);
  };

  /**
   * Handles click events while drawing a polyline.
   * @param {Cartesian2} clickPosition The click position
   */
  HorizontalMeasurementDrawing.prototype.handleClick = function (clickPosition) {
      if (this._mode === DrawingMode.AfterDraw) {
          this.reset();
      }
      var position = HorizontalPolylineDrawing.prototype.handleClick.call(this, clickPosition);
      if (cesium.defined(position)) {
          this._label.show = true;
          this._polyline.show = true;
      }
  };

  /**
   * Handles mouse movements while drawing a horizontal measurement.
   * @param {Cartesian2} mousePosition The mouse position
   * @param {Boolean} shift True if the shift key was pressed
   */
  HorizontalMeasurementDrawing.prototype.handleMouseMove = function (mousePosition, shift) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('mousePosition', mousePosition);
      cesium.Check.defined('shift', shift);
      //>>includeEnd('debug');

      var nextPos = HorizontalPolylineDrawing.prototype.handleMouseMove.call(this, mousePosition, shift);
      if (!cesium.defined(nextPos)) {
          return;
      }

      var positions = this._positions;
      var lastPos = positions[positions.length - 1];
      var vec = cesium.Cartesian3.subtract(nextPos, lastPos, v1Scratch$2);
      var distance = this._previousDistance + cesium.Cartesian3.magnitude(vec);

      var label = this._label;
      label.position = cesium.Cartesian3.midpoint(lastPos, nextPos, cart3Scratch$3);
      label.text = MeasureUnits.distanceToString(distance, this._selectedUnits.distanceUnits, this._selectedLocale);
      label.show = true;

      this._distance = distance;
  };

  /**
   * Resets the measurement.
   */
  HorizontalMeasurementDrawing.prototype.reset = function () {
      var i;
      var primitives = this._primitives;
      var dashLines = this._dashedLines;
      for (i = 0; i < dashLines.length; i++) {
          primitives.remove(dashLines[i]);
      }
      this._dashedLines = [];

      this._polyline.positions = [];
      this._polyline.show = false;

      this._label.show = false;
      this._label.text = '';

      this._previousDistance = 0;
      this._distance = 0;

      this._positions = [];

      var points = this._points;
      var pointCollection = this._pointCollection;
      for (i = 0; i < points.length; i++) {
          pointCollection.remove(points[i]);
      }
      points.length = 0;

      var labelCollection = this._labelCollection;
      var labels = this._segmentLabels;
      for (i = 0; i < labels.length; i++) {
          labelCollection.remove(labels[i]);
      }
      labels.length = 0;

      this._moveDashLine.show = false;
      this._mode = DrawingMode.BeforeDraw;
      this._lastClickPosition.x = Number.POSITIVE_INFINITY;
      this._lastClickPosition.y = Number.POSITIVE_INFINITY;
  };

  /**
   * Destroys the measurement.
   */
  HorizontalMeasurementDrawing.prototype.destroy = function () {
      this._removeEvent();

      var i;
      var labelCollection = this._labelCollection;
      var labels = this._segmentLabels;
      for (i = 0; i < labels.length; i++) {
          labelCollection.remove(labels[i]);
      }

      var primitives = this._primitives;
      var dashLines = this._dashedLines;
      for (i = 0; i < dashLines.length; i++) {
          primitives.remove(dashLines[i]);
      }

      this._labelCollection.remove(this._label);

      HorizontalPolylineDrawing.prototype.destroy.call(this);
  };

  function getIcon$3(size) {
      return '<svg viewBox="0 0 30 30" height="' + size + 'px" width="' + size + 'px">\n\
                 <g transform="translate(0,-267)">\n\
                   <path d="m 5.5492003,281.78808 18.9375757,0.0497"/>\n\
                   <circle r="2.0788691" cy="281.63776" cx="3.0514872"/>\n\
                   <circle r="2.0788691" cy="281.71384" cx="26.985731"/>\n\
                 </g>\n\
               </svg>';
  }

  function HorizontalMeasurement(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      Measurement.call(this, options);

      this._drawing = new HorizontalMeasurementDrawing(options);
  }

  HorizontalMeasurement.prototype = Object.create(Measurement.prototype);
  HorizontalMeasurement.prototype.constructor = HorizontalMeasurement;

  Object.defineProperties(HorizontalMeasurement.prototype, {
      distance: {
          get: function () {
              return this._drawing.distance;
          }
      },
      icon: {
          value: getIcon$3(15)
      },
      thumbnail: {
          value: getIcon$3(25)
      },
      type: {
          value: 'Horizontal distance'
      },
      instructions: {
          value: ['Click on the point cloud or the globe to set the start point', 'Move the mouse to drag the line', 'Press this shift key to clamp the direction of the line', 'Click again to set the end point', 'To make a new measurement, click to clear the previous measurement']
      },

      id: {
          value: 'horizontalMeasurement'
      }
  });

  HorizontalMeasurement.prototype.handleDoubleClick = function () {
      this._drawing.handleDoubleClick();
  };

  HorizontalMeasurement.prototype.handleClick = function (clickPosition) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('clickPosition', clickPosition);
      //>>includeEnd('debug');

      this._drawing.handleClick(clickPosition);
  };

  HorizontalMeasurement.prototype.handleMouseMove = function (mousePosition, shift) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('mousePosition', mousePosition);
      cesium.Check.defined('shift', shift);
      //>>includeEnd('debug');

      this._drawing.handleMouseMove(mousePosition, shift);
  };

  HorizontalMeasurement.prototype.reset = function () {
      this._drawing.reset();
  };

  HorizontalMeasurement.prototype.isDestroyed = function () {
      return false;
  };

  HorizontalMeasurement.prototype.destroy = function () {
      this._drawing.destroy();

      return cesium.destroyObject(this);
  };

  function MeasurementMouseHandler(scene) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('scene', scene);
      //>>includeEnd('debug');

      this.selectedMeasurement = undefined;
      this._sseh = new cesium.ScreenSpaceEventHandler(scene.canvas);
      this._scene = scene;
  }

  Object.defineProperties(MeasurementMouseHandler.prototype, {
      scene: {
          get: function () {
              return this._scene;
          }
      }
  });

  MeasurementMouseHandler.prototype.activate = function () {
      var sseh = this._sseh;
      sseh.setInputAction(this._click.bind(this), cesium.ScreenSpaceEventType.LEFT_CLICK);
      sseh.setInputAction(this._clickShift.bind(this), cesium.ScreenSpaceEventType.LEFT_CLICK, cesium.KeyboardEventModifier.SHIFT);
      sseh.setInputAction(this._mouseMove.bind(this), cesium.ScreenSpaceEventType.MOUSE_MOVE);
      sseh.setInputAction(this._mouseMoveShift.bind(this), cesium.ScreenSpaceEventType.MOUSE_MOVE, cesium.KeyboardEventModifier.SHIFT);
      sseh.setInputAction(this._handleLeftDown.bind(this), cesium.ScreenSpaceEventType.LEFT_DOWN);
      sseh.setInputAction(this._handleLeftUp.bind(this), cesium.ScreenSpaceEventType.LEFT_UP);
      sseh.setInputAction(this._handleDoubleClick.bind(this), cesium.ScreenSpaceEventType.LEFT_DOUBLE_CLICK);
  };

  MeasurementMouseHandler.prototype.deactivate = function () {
      var sseh = this._sseh;
      sseh.removeInputAction(cesium.ScreenSpaceEventType.LEFT_CLICK);
      sseh.removeInputAction(cesium.ScreenSpaceEventType.LEFT_CLICK, cesium.KeyboardEventModifier.SHIFT);
      sseh.removeInputAction(cesium.ScreenSpaceEventType.MOUSE_MOVE);
      sseh.removeInputAction(cesium.ScreenSpaceEventType.MOUSE_MOVE, cesium.KeyboardEventModifier.SHIFT);
      sseh.removeInputAction(cesium.ScreenSpaceEventType.LEFT_DOWN);
      sseh.removeInputAction(cesium.ScreenSpaceEventType.LEFT_UP);
      sseh.removeInputAction(cesium.ScreenSpaceEventType.LEFT_DOUBLE_CLICK);
  };

  MeasurementMouseHandler.prototype._handleDoubleClick = function (click) {
      this.selectedMeasurement.handleDoubleClick(click.position);
      var scene = this._scene;
      if (scene.requestRenderMode) {
          scene.requestRender();
      }
  };

  MeasurementMouseHandler.prototype._handleClick = function (click, shift) {
      this.selectedMeasurement.handleClick(click.position, shift);
      var scene = this._scene;
      if (scene.requestRenderMode) {
          scene.requestRender();
      }
  };

  MeasurementMouseHandler.prototype._clickShift = function (click) {
      this._handleClick(click, true);
  };

  MeasurementMouseHandler.prototype._click = function (click) {
      this._handleClick(click, false);
  };

  MeasurementMouseHandler.prototype._handleMouseMove = function (movement, shift) {
      this.selectedMeasurement.handleMouseMove(movement.endPosition, shift);
      var scene = this._scene;
      if (scene.requestRenderMode) {
          scene.requestRender();
      }
  };

  MeasurementMouseHandler.prototype._mouseMove = function (movement) {
      this._handleMouseMove(movement, false);
  };

  MeasurementMouseHandler.prototype._mouseMoveShift = function (movement) {
      this._handleMouseMove(movement, true);
  };

  MeasurementMouseHandler.prototype._handleLeftDown = function (event) {
      this.selectedMeasurement.handleLeftDown(event.position);
      var scene = this._scene;
      if (scene.requestRenderMode) {
          scene.requestRender();
      }
  };

  MeasurementMouseHandler.prototype._handleLeftUp = function (event) {
      this.selectedMeasurement.handleLeftUp(event.position);
      var scene = this._scene;
      if (scene.requestRenderMode) {
          scene.requestRender();
      }
  };

  /**
   * @returns {Boolean} true if the object has been destroyed, false otherwise.
   */
  MeasurementMouseHandler.prototype.isDestroyed = function () {
      return false;
  };

  MeasurementMouseHandler.prototype.destroy = function () {
      this.deactivate();
      this._sseh.destroy();
      return cesium.destroyObject(this);
  };

  var positionScratch = new cesium.Cartesian3();
  var normalScratch$1 = new cesium.Cartesian3();
  var surfaceNormalScratch = new cesium.Cartesian3();

  var scratchCartesian2s = [new cesium.Cartesian2(), new cesium.Cartesian2(), new cesium.Cartesian2(), new cesium.Cartesian2()];
  var scratchCartesian3s = [new cesium.Cartesian3(), new cesium.Cartesian3(), new cesium.Cartesian3(), new cesium.Cartesian3(), new cesium.Cartesian3()];

  function getSlope(scene, windowCoordinates) {
      cesium.Check.defined('scene', scene);
      cesium.Check.defined('windowCoordinates', windowCoordinates);

      var worldPosition = getSlope._getWorldPosition(scene, windowCoordinates, positionScratch);
      if (!cesium.defined(worldPosition)) {
          return;
      }

      var distanceCameraToPositionThreshold = 10000.0;
      var pixelOffset = 2;
      var offsetDistanceRatioThreshold = 0.05;

      var cameraPosition = scene.camera.position;
      var distanceCameraToPosition = cesium.Cartesian3.distance(worldPosition, cameraPosition);

      if (distanceCameraToPosition > distanceCameraToPositionThreshold) {
          // don't compute slope if camera is more than 10km away from point
          return;
      }

      var sc0 = scratchCartesian3s[0];
      var sc1 = scratchCartesian3s[1];
      var sc2 = scratchCartesian3s[2];
      var sc3 = scratchCartesian3s[3];

      var normal = scene.frameState.mapProjection.ellipsoid.geodeticSurfaceNormal(worldPosition, normalScratch$1);
      normal = cesium.Cartesian3.negate(normal, normal);

      var sampledWindowCoordinate0 = cesium.Cartesian2.clone(windowCoordinates, scratchCartesian2s[0]);
      sampledWindowCoordinate0.x -= pixelOffset;
      sampledWindowCoordinate0.y -= pixelOffset;

      var sampledWindowCoordinate1 = cesium.Cartesian2.clone(windowCoordinates, scratchCartesian2s[1]);
      sampledWindowCoordinate1.x -= pixelOffset;
      sampledWindowCoordinate1.y += pixelOffset;

      var sampledWindowCoordinate2 = cesium.Cartesian2.clone(windowCoordinates, scratchCartesian2s[2]);
      sampledWindowCoordinate2.x += pixelOffset;
      sampledWindowCoordinate2.y += pixelOffset;

      var sampledWindowCoordinate3 = cesium.Cartesian2.clone(windowCoordinates, scratchCartesian2s[3]);
      sampledWindowCoordinate3.x += pixelOffset;
      sampledWindowCoordinate3.y -= pixelOffset;

      var sPosition0 = getSlope._getWorldPosition(scene, sampledWindowCoordinate0, sc0);
      var sPosition1 = getSlope._getWorldPosition(scene, sampledWindowCoordinate1, sc1);
      var sPosition2 = getSlope._getWorldPosition(scene, sampledWindowCoordinate2, sc2);
      var sPosition3 = getSlope._getWorldPosition(scene, sampledWindowCoordinate3, sc3);

      var v0, v1, v2, v3;
      if (cesium.defined(sPosition0)) {
          var line0 = cesium.Cartesian3.subtract(sPosition0, worldPosition, sc0);
          var d0 = cesium.Cartesian3.magnitude(line0);
          v0 = d0 / distanceCameraToPosition <= offsetDistanceRatioThreshold ? cesium.Cartesian3.normalize(line0, sc0) : undefined;
      }

      if (cesium.defined(sPosition1)) {
          var line1 = cesium.Cartesian3.subtract(sPosition1, worldPosition, sc1);
          var d1 = cesium.Cartesian3.magnitude(line1);
          v1 = d1 / distanceCameraToPosition <= offsetDistanceRatioThreshold ? cesium.Cartesian3.normalize(line1, sc1) : undefined;
      }

      if (cesium.defined(sPosition2)) {
          var line2 = cesium.Cartesian3.subtract(sPosition2, worldPosition, sc2);
          var d2 = cesium.Cartesian3.magnitude(line2);
          v2 = d2 / distanceCameraToPosition <= offsetDistanceRatioThreshold ? cesium.Cartesian3.normalize(line2, sc2) : undefined;
      }

      if (cesium.defined(sPosition3)) {
          var line3 = cesium.Cartesian3.subtract(sPosition3, worldPosition, sc3);
          var d3 = cesium.Cartesian3.magnitude(line3);
          v3 = d3 / distanceCameraToPosition <= offsetDistanceRatioThreshold ? cesium.Cartesian3.normalize(line3, sc3) : undefined;
      }

      var surfaceNormal = cesium.Cartesian3.clone(cesium.Cartesian3.ZERO, surfaceNormalScratch);
      var scratchNormal = scratchCartesian3s[4];

      if (cesium.defined(v0) && cesium.defined(v1)) {
          scratchNormal = cesium.Cartesian3.normalize(cesium.Cartesian3.cross(v0, v1, scratchNormal), scratchNormal);
          surfaceNormal = cesium.Cartesian3.add(surfaceNormal, scratchNormal, surfaceNormal);
      }
      if (cesium.defined(v1) && cesium.defined(v2)) {
          scratchNormal = cesium.Cartesian3.normalize(cesium.Cartesian3.cross(v1, v2, scratchNormal), scratchNormal);
          surfaceNormal = cesium.Cartesian3.add(surfaceNormal, scratchNormal, surfaceNormal);
      }
      if (cesium.defined(v2) && cesium.defined(v3)) {
          scratchNormal = cesium.Cartesian3.normalize(cesium.Cartesian3.cross(v2, v3, scratchNormal), scratchNormal);
          surfaceNormal = cesium.Cartesian3.add(surfaceNormal, scratchNormal, surfaceNormal);
      }
      if (cesium.defined(v3) && cesium.defined(v0)) {
          scratchNormal = cesium.Cartesian3.normalize(cesium.Cartesian3.cross(v3, v0, scratchNormal), scratchNormal);
          surfaceNormal = cesium.Cartesian3.add(surfaceNormal, scratchNormal, surfaceNormal);
      }

      if (surfaceNormal.equals(cesium.Cartesian3.ZERO)) {
          return;
      }

      surfaceNormal = cesium.Cartesian3.normalize(surfaceNormal, surfaceNormal);

      return cesium.Math.asinClamped(Math.abs(Math.sin(cesium.Cartesian3.angleBetween(surfaceNormal, normal)))); // Always between 0 and PI/2.
  }

  // exposed for specs
  getSlope._getWorldPosition = getWorldPosition;

  var scratchCartesian = new cesium.Cartesian3();
  var scratchCartographic = new cesium.Cartographic();

  function getIcon$4(size) {
      return '<svg viewBox="0 0 30 30" height="' + size + 'px" width="' + size + 'px">\n\
                 <g transform="translate(0,-267)">\n\
                   <circle r="2.0788691" cy="281.90503" cx="15.212251"/>\n\
                 </g>\n\
               </svg>';
  }

  function PointMeasurement(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      Measurement.call(this, options);

      this._point = this._pointCollection.add(MeasurementSettings.getPointOptions());
      this._label = this._labelCollection.add(MeasurementSettings.getLabelOptions({
          horizontalOrigin: cesium.HorizontalOrigin.LEFT,
          verticalOrigin: cesium.VerticalOrigin.CENTER,
          pixelOffset: new cesium.Cartesian2(10, 0)
      }));
      this._position = new cesium.Cartesian3();
      this._height = 0.0;
      this._slope = 0.0;
  }

  PointMeasurement.prototype = Object.create(Measurement.prototype);
  PointMeasurement.prototype.constructor = PointMeasurement;

  Object.defineProperties(PointMeasurement.prototype, {
      /**
       * Gets the position.
       * @type {Cartesian3}
       * @memberof PointMeasurement.prototype
       * @readonly
       */
      position: {
          get: function () {
              return this._position;
          }
      },
      /**
       * Gets the height.
       * @type {Number}
       * @memberof PointMeasurement.prototype
       * @readonly
       */
      height: {
          get: function () {
              return this._height;
          }
      },
      /**
       * Gets the slope in radians.
       * @type {Number}
       * @memberof PointMeasurement.prototype
       * @readonly
       */
      slope: {
          get: function () {
              return this._slope;
          }
      },
      /**
       * Gets the icon.
       * @type {String}
       * @memberof PointMeasurement.prototype
       * @readonly
       */
      icon: {
          value: getIcon$4(15)
      },
      /**
       * Gets the thumbnail.
       * @type {String}
       * @memberof PointMeasurement.prototype
       * @readonly
       */
      thumbnail: {
          value: getIcon$4(25)
      },
      /**
       * Gets the type.
       * @type {String}
       * @memberof PointMeasurement.prototype
       * @readonly
       */
      type: {
          value: 'Point coordinates'
      },
      /**
       * Gets the instruction text.
       * @type {String[]}
       * @memberof PointMeasurement.prototype
       * @readonly
       */
      instructions: {
          value: ['Move the mouse to see the longitude, latitude and height of the point']
      },
      /**
       * Gets the id.
       * @type {String}
       * @memberof PointMeasurement.prototype
       * @readonly
       */
      id: {
          value: 'pointMeasurement'
      }
  });

  PointMeasurement.prototype._pickPositionSupported = function () {
      return this._scene.pickPositionSupported;
  };

  /**
   * Handles drawing on mouse move.
   */
  PointMeasurement.prototype.handleMouseMove = function (movePosition) {
      var scene = this._scene;
      this.reset();

      if (scene.mode === cesium.SceneMode.MORPHING) {
          return;
      }

      this._point.show = false;

      var position = PointMeasurement._getWorldPosition(scene, movePosition, scratchCartesian);
      if (!cesium.defined(position)) {
          return;
      }

      this._point.position = position;

      var positionCartographic = scene.frameState.mapProjection.ellipsoid.cartesianToCartographic(position, scratchCartographic);
      var height = 0.0;
      if (cesium.defined(scene.globe)) {
          height = cesium.defaultValue(scene.globe.getHeight(positionCartographic), 0.0);
      }
      height = positionCartographic.height - height;
      if (cesium.Math.equalsEpsilon(height, 0.0, cesium.Math.EPSILON3)) {
          height = 0.0;
      }

      var slope;
      if (scene.mode !== cesium.SceneMode.SCENE2D) {
          slope = PointMeasurement._getSlope(scene, movePosition, this._primitives);
      }

      this._point.show = true;

      var label = this._label;
      label.position = position;
      label.show = true;
      label.text = 'lon: ' + MeasureUnits.angleToString(positionCartographic.longitude, AngleUnits.DEGREES_MINUTES_SECONDS, this._selectedLocale) + '\n' + 'lat: ' + MeasureUnits.angleToString(positionCartographic.latitude, AngleUnits.DEGREES_MINUTES_SECONDS, this._selectedLocale);

      if (scene.mode !== cesium.SceneMode.SCENE2D && this._pickPositionSupported()) {
          label.text += '\nheight: ' + MeasureUnits.distanceToString(height, this._selectedUnits.distanceUnits, this._selectedLocale);
          if (cesium.defined(slope)) {
              label.text += '\nslope: ' + MeasureUnits.angleToString(slope, this._selectedUnits.slopeUnits, this._selectedLocale, 3);
          }
      }

      this._position = cesium.Cartesian3.clone(position, this._position);
      this._height = height;
      this._slope = slope;
  };

  /**
   * Resets the widget.
   */
  PointMeasurement.prototype.reset = function () {
      this._label.show = false;
      this._point.show = false;
      this._position = cesium.Cartesian3.clone(cesium.Cartesian3.ZERO, this._position);
  };

  /**
   * @returns {Boolean} true if the object has been destroyed, false otherwise.
   */
  PointMeasurement.prototype.isDestroyed = function () {
      return false;
  };

  /**
   * Destroys the measurement.
   */
  PointMeasurement.prototype.destroy = function () {
      this._labelCollection.remove(this._label);
      this._pointCollection.remove(this._point);

      return cesium.destroyObject(this);
  };

  // exposed for specs
  PointMeasurement._getSlope = getSlope;
  PointMeasurement._getWorldPosition = getWorldPosition;

  var cart2Scratch1$3 = new cesium.Cartesian2();
  var cart2Scratch2$3 = new cesium.Cartesian2();
  var cart2Scratch3$1 = new cesium.Cartesian2();

  var scratch$1 = new cesium.Cartesian3();

  function PolylineMeasurementDrawing(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('options.scene', options.scene);
      cesium.Check.defined('options.primitives', options.primitives);
      cesium.Check.defined('options.units', options.units);
      cesium.Check.defined('options.points', options.points);
      cesium.Check.defined('options.labels', options.labels);
      //>>includeEnd('debug');

      options.polylineOptions = MeasurementSettings.getPolylineOptions({
          ellipsoid: options.ellipsoid
      });
      options.pointOptions = MeasurementSettings.getPointOptions();
      PolylineDrawing.call(this, options);

      var scene = this._scene;
      var labels = options.labels;
      this._labelCollection = labels;
      this._label = labels.add(MeasurementSettings.getLabelOptions());
      this._segmentLabels = [];
      this._selectedUnits = options.units;
      this._selectedLocale = options.locale;
      this._previousDistance = 0;
      this._distance = 0;

      var that = this;
      this._removeEvent = scene.preRender.addEventListener(function () {
          that.updateLabel();
      });
  }

  PolylineMeasurementDrawing.prototype = Object.create(PolylineDrawing.prototype);
  PolylineMeasurementDrawing.prototype.constructor = PolylineMeasurementDrawing;

  Object.defineProperties(PolylineMeasurementDrawing.prototype, {
      /**
       * Gets the distance in meters.
       * @type {Number}
       * @memberof PolylineMeasurementDrawing.prototype
       * @readonly
       */
      distance: {
          get: function () {
              return this._distance;
          }
      }
  });

  /**
   * Updates the label position.
   * @private
   */
  PolylineMeasurementDrawing.prototype.updateLabel = function () {
      var positions = this._positions;
      if (positions.length < 2) {
          return;
      }
      var scene = this._scene;
      var top = positions[0];
      var pos2d = cesium.SceneTransforms.wgs84ToWindowCoordinates(scene, top, cart2Scratch1$3);
      var lastScreenPos = cesium.defined(pos2d) ? cesium.Cartesian2.clone(pos2d, cart2Scratch3$1) : cesium.Cartesian2.fromElements(Number.NEGATIVE_INFINITY, Number.POSITIVE_INFINITY, cart2Scratch3$1);
      var topY = lastScreenPos.y;
      var labels = this._segmentLabels;
      labels[0].show = this._polyline.positions.length > 2;
      for (var i = 1; i < positions.length; i++) {
          var nextScreenPos = cesium.SceneTransforms.wgs84ToWindowCoordinates(scene, positions[i], cart2Scratch2$3);
          if (!cesium.defined(nextScreenPos)) {
              continue;
          }

          var m = (lastScreenPos.y - nextScreenPos.y) / (nextScreenPos.x - lastScreenPos.x);
          var label = labels[i - 1];
          if (m > 0) {
              label.horizontalOrigin = cesium.HorizontalOrigin.LEFT;
          } else {
              label.horizontalOrigin = cesium.HorizontalOrigin.RIGHT;
          }

          if (nextScreenPos.y < topY) {
              topY = nextScreenPos.y;
              top = positions[i];
          }
          lastScreenPos = cesium.Cartesian2.clone(nextScreenPos, lastScreenPos);
      }
      if (this._mode === DrawingMode.AfterDraw) {
          this._label.position = top;
      }
  };

  /**
   * Adds a point to the polyline.
   * @param {Cartesian3} position The position to add
   * @private
   */
  PolylineMeasurementDrawing.prototype.addPoint = function (position) {
      var positions = this._positions;
      if (positions.length > 0) {
          // store distance that was calculated on mouse move
          this._previousDistance = this._distance;

          var label = this._labelCollection.add(MeasurementSettings.getLabelOptions({
              scale: 0.8,
              horizontalOrigin: cesium.HorizontalOrigin.LEFT,
              verticalOrigin: cesium.VerticalOrigin.TOP,
              pixelOffset: new cesium.Cartesian2(5, 5)
          }));
          var p1 = positions[positions.length - 1];
          label.position = cesium.Cartesian3.midpoint(p1, position, new cesium.Cartesian3());
          label.text = MeasureUnits.distanceToString(cesium.Cartesian3.distance(p1, position), this._selectedUnits.distanceUnits, this._selectedLocale);
          label.show = true;
          this._segmentLabels.push(label);
      }
      PolylineDrawing.prototype.addPoint.call(this, position);
  };

  /**
   * Handles click events while drawing a polyline.
   * @param {Cartesian2} clickPosition The click position
   */
  PolylineMeasurementDrawing.prototype.handleClick = function (clickPosition) {
      if (this._mode === DrawingMode.AfterDraw) {
          this.reset();
      }
      var position = PolylineDrawing.prototype.handleClick.call(this, clickPosition);
      if (cesium.defined(position)) {
          this._label.show = true;
          this._polyline.show = true;
      }
  };

  /**
   * Handles mouse move events while drawing a polyline.
   * @param {Cartesian2} mousePosition The mouse position
   */
  PolylineMeasurementDrawing.prototype.handleMouseMove = function (mousePosition) {
      var nextPos = PolylineDrawing.prototype.handleMouseMove.call(this, mousePosition);
      if (!cesium.defined(nextPos)) {
          return;
      }

      var pos1 = this._positions[this._positions.length - 1];
      var pos2 = nextPos;
      var vec = cesium.Cartesian3.subtract(pos2, pos1, scratch$1);
      var distance = this._previousDistance + cesium.Cartesian3.magnitude(vec);

      var label = this._label;
      label.position = pos2;
      label.text = MeasureUnits.distanceToString(distance, this._selectedUnits.distanceUnits, this._selectedLocale);
      label.show = true;

      this._distance = distance;
  };

  /**
   * Resets the widget.
   */
  PolylineMeasurementDrawing.prototype.reset = function () {
      this._label.show = false;
      this._label.text = '';
      this._positions = [];
      this._polyline.positions = [];
      this._polyline.show = false;
      this._previousDistance = 0;
      this._distance = 0;

      var i;
      var points = this._points;
      var pointCollection = this._pointCollection;
      for (i = 0; i < points.length; i++) {
          pointCollection.remove(points[i]);
      }
      points.length = 0;

      var labelCollection = this._labelCollection;
      var labels = this._segmentLabels;
      for (i = 0; i < labels.length; i++) {
          labelCollection.remove(labels[i]);
      }
      labels.length = 0;

      this._mode = DrawingMode.BeforeDraw;
      this._lastClickPosition.x = Number.POSITIVE_INFINITY;
      this._lastClickPosition.y = Number.POSITIVE_INFINITY;
  };

  /**
   * Destroys the widget.
   */
  PolylineMeasurementDrawing.prototype.destroy = function () {
      this._removeEvent();

      var labelCollection = this._labelCollection;
      labelCollection.remove(this._label);
      var labels = this._segmentLabels;
      for (var i = 0; i < labels.length; i++) {
          labelCollection.remove(labels[i]);
      }

      PolylineDrawing.prototype.destroy.call(this);
  };

  function getIcon$5(size) {
      return '<svg viewBox="0 0 30 30" height="' + size + 'px" width="' + size + 'px">\n\
                  <g transform="translate(0,-267)">\n\
                   <circle cx="3.8532958" cy="293.99896" r="2.0788691" />\n\
                   <circle cx="7.2651663" cy="276.26389" r="2.0788691" />\n\
                   <circle cx="24.571842" cy="285.56577" r="2.0788691" />\n\
                   <circle cx="26.916754" cy="270.38345" r="2.0788691" />\n\
                   <path d="m 3.7523356,294.14823 3.602242,-17.81109 17.3608064,9.35582 2.401494,-15.00934" />\n\
                 </g>\n\
               </svg>';
  }

  function PolylineMeasurement(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      Measurement.call(this, options);

      this._drawing = new PolylineMeasurementDrawing(options);
  }

  PolylineMeasurement.prototype = Object.create(Measurement.prototype);
  PolylineMeasurement.prototype.constructor = PolylineMeasurement;

  Object.defineProperties(PolylineMeasurement.prototype, {
      /**
       * Gets the distance in meters.
       * @type {Number}
       * @memberof PolylineMeasurement.prototype
       * @readonly
       */
      distance: {
          get: function () {
              return this._drawing.distance;
          }
      },
      /**
       * Gets the icon.
       * @type {String}
       * @memberof PolylineMeasurement.prototype
       * @readonly
       */
      icon: {
          value: getIcon$5(15)
      },
      /**
       * Gets the thumbnail.
       * @type {String}
       * @memberof PolylineMeasurement.prototype
       * @readonly
       */
      thumbnail: {
          value: getIcon$5(25)
      },
      /**
       * Gets the type.
       * @type {String}
       * @memberof PolylineMeasurement.prototype
       * @readonly
       */
      type: {
          value: 'Polyline Distance'
      },
      /**
       * Gets the instruction text.
       * @type {String[]}
       * @memberof PolylineMeasurement.prototype
       * @readonly
       */
      instructions: {
          value: ['Click to start drawing a polyline', 'Keep clicking to add more points', 'Double click to finish drawing']
      },
      /**
       * Gets the id.
       * @type {String}
       * @memberof PolylineMeasurement.prototype
       * @readonly
       */
      id: {
          value: 'polylineMeasurement'
      }
  });

  /**
   * Ends drawing on double click.
   */
  PolylineMeasurement.prototype.handleDoubleClick = function () {
      this._drawing.handleDoubleClick();
  };

  /**
   * Handles click events while drawing a polyline.
   * @param {Cartesian2} clickPosition The click position
   */
  PolylineMeasurement.prototype.handleClick = function (clickPosition) {
      this._drawing.handleClick(clickPosition);
  };

  /**
   * Handles mouse move events while drawing a polyline.
   * @param {Cartesian2} mousePosition The mouse position
   */
  PolylineMeasurement.prototype.handleMouseMove = function (mousePosition) {
      this._drawing.handleMouseMove(mousePosition);
  };

  /**
   * Resets the widget.
   */
  PolylineMeasurement.prototype.reset = function () {
      this._drawing.reset();
  };

  /**
   * @returns {Boolean} true if the object has been destroyed, false otherwise.
   */
  PolylineMeasurement.prototype.isDestroyed = function () {
      return false;
  };

  /**
   * Destroys the widget.
   */
  PolylineMeasurement.prototype.destroy = function () {
      this._drawing.destroy();

      return cesium.destroyObject(this);
  };

  var Mode$1 = {
      BeforeDraw: 0,
      Drawing: 1,
      AfterDraw: 2
  };

  var scratch$2 = new cesium.Cartesian3();
  var cart2 = new cesium.Cartesian2();
  var normalScratch$2 = new cesium.Cartesian3();
  var v1 = new cesium.Cartesian3();
  var rayScratch$2 = new cesium.Ray();
  var positionScratch$1 = new cesium.Cartesian3();
  var scratchCarto$2 = new cesium.Cartographic();

  function getIcon$6(size) {
      return '<svg viewBox="0 0 30 30" height="' + size + 'px" width="' + size + 'px">\n\
                 <g transform="translate(0,-267)">\n\
                   <path d="m 15.042838,272.34414 -0.0497,18.93758"/>\n\
                   <circle r="2.0788691" cy="270.01154" cx="15.078616"/>\n\
                   <circle r="2.0788691" cy="293.97095" cx="15.092237"/>\n\
                 </g>\n\
               </svg>';
  }

  function getHeightPosition(measurement, mousePos) {
      var positions = measurement._positions;
      var pos0 = positions[0];
      var pos1 = positions[1];
      var plane = measurement._draggingPlane;
      var normal = measurement._surfaceNormal;
      var scene = measurement._scene;
      var camera = scene.camera;
      var cameraDirection = camera.direction;
      var ellipsoid = scene.frameState.mapProjection.ellipsoid;

      var planePoint = pos0;
      var surfaceNormal = normal;

      if (scene.mode === cesium.SceneMode.COLUMBUS_VIEW) {
          surfaceNormal = cesium.Cartesian3.UNIT_X;
          var cartoPos = ellipsoid.cartesianToCartographic(pos0, scratchCarto$2);
          planePoint = scene.mapProjection.project(cartoPos, scratch$2);
          cesium.Cartesian3.fromElements(planePoint.z, planePoint.x, planePoint.y, planePoint);
      }

      var planeNormal = cesium.Cartesian3.cross(surfaceNormal, cameraDirection, normalScratch$2);
      planeNormal = cesium.Cartesian3.cross(surfaceNormal, planeNormal, planeNormal);
      planeNormal = cesium.Cartesian3.normalize(planeNormal, planeNormal);
      plane = cesium.Plane.fromPointNormal(planePoint, planeNormal, plane);
      var ray = camera.getPickRay(mousePos, rayScratch$2);

      pos1 = cesium.IntersectionTests.rayPlane(ray, plane, pos1);
      if (!cesium.defined(pos1)) {
          return;
      }

      if (scene.mode === cesium.SceneMode.COLUMBUS_VIEW) {
          pos1 = cesium.Cartesian3.fromElements(pos1.y, pos1.z, pos1.x, pos1);
          var carto = scene.mapProjection.unproject(pos1, scratchCarto$2);
          pos1 = ellipsoid.cartographicToCartesian(carto, pos1);
      }

      var screenPos = cesium.SceneTransforms.wgs84ToWindowCoordinates(scene, positions[0], cart2);
      if (screenPos.y < mousePos.y) {
          normal = cesium.Cartesian3.negate(normal, normalScratch$2);
      }
      v1 = cesium.Cartesian3.subtract(pos1, pos0, v1);
      v1 = cesium.Cartesian3.projectVector(v1, normal, v1);
      pos1 = cesium.Cartesian3.add(pos0, v1, pos1);
      return pos1;
  }

  function VerticalMeasurement(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      Measurement.call(this, options);

      var pointCollection = this._pointCollection;
      var positions = [new cesium.Cartesian3(), new cesium.Cartesian3()];

      this._startPoint = pointCollection.add(MeasurementSettings.getPointOptions());
      this._endPoint = pointCollection.add(MeasurementSettings.getPointOptions());

      this._positions = positions;
      this._polyline = this._primitives.add(new PolylinePrimitive(MeasurementSettings.getPolylineOptions({
          ellipsoid: this._scene.frameState.mapProjection.ellipsoid,
          positions: positions
      })));

      this._label = this._labelCollection.add(MeasurementSettings.getLabelOptions({
          horizontalOrigin: cesium.HorizontalOrigin.LEFT,
          verticalOrigin: cesium.VerticalOrigin.TOP,
          pixelOffset: new cesium.Cartesian2(10, 10)
      }));

      this._mode = Mode$1.BeforeDraw;
      this._draggingPlane = new cesium.Plane(cesium.Cartesian3.UNIT_X, 0);
      this._surfaceNormal = new cesium.Cartesian3();
      this._distance = 0;
  }

  VerticalMeasurement.prototype = Object.create(Measurement.prototype);
  VerticalMeasurement.prototype.constructor = VerticalMeasurement;

  Object.defineProperties(VerticalMeasurement.prototype, {
      /**
       * Gets the distance.
       * @type {Number}
       * @memberof VerticalMeasurement.prototype
       * @readonly
       */
      distance: {
          get: function () {
              return this._distance;
          }
      },
      /**
       * Gets the type.
       * @type {String}
       * @memberof VerticalMeasurement.prototype
       * @readonly
       */
      type: {
          value: 'Vertical distance'
      },
      /**
       * Gets the icon.
       * @type {String}
       * @memberof VerticalMeasurement.prototype
       * @readonly
       */
      icon: {
          value: getIcon$6(15)
      },
      /**
       * Gets the thumbnail.
       * @type {String}
       * @memberof VerticalMeasurement.prototype
       * @readonly
       */
      thumbnail: {
          value: getIcon$6(25)
      },
      /**
       * Gets the instruction text.
       * @type {String[]}
       * @memberof VerticalMeasurement.prototype
       * @readonly
       */
      instructions: {
          value: ['Click on the point cloud or the globe to set the start point', 'Move the mouse to drag the line', 'Click again to set the end point', 'To make a new measurement, click to clear the previous measurement']
      },
      /**
       * Gets the id.
       * @type {String}
       * @memberof VerticalMeasurement.prototype
       * @readonly
       */
      id: {
          value: 'verticalMeasurement'
      }
  });

  /**
   * Handles click events while drawing a vertical measurement.
   * @param {Cartesian2} clickPosition The click position
   */
  VerticalMeasurement.prototype.handleClick = function (clickPosition) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('clickPosition', clickPosition);
      //>>includeEnd('debug');

      var scene = this._scene;
      var ellipsoid = scene.frameState.mapProjection.ellipsoid;
      if (this._mode === Mode$1.AfterDraw) {
          this.reset();
      }

      var mode = this._mode;
      var positions = this._positions;
      if (mode === Mode$1.BeforeDraw) {
          var pos = VerticalMeasurement._getWorldPosition(scene, clickPosition, positions[0]);
          if (!cesium.defined(pos)) {
              return;
          }
          this._polyline.show = true;
          positions[0] = cesium.Cartesian3.clone(pos, positions[0]);
          positions[1] = cesium.Cartesian3.clone(pos, positions[1]);
          this._startPoint.position = pos;
          this._startPoint.show = true;
          this._mode = Mode$1.Drawing;
          this._polyline.positions = positions;
          this._surfaceNormal = ellipsoid.geodeticSurfaceNormal(pos, this._surfaceNormal);
      } else if (mode === Mode$1.Drawing) {
          this._endPoint.position = positions[1];
          this._endPoint.show = true;
          this._mode = Mode$1.AfterDraw;
      }
  };

  /**
   * Handles mouse movement while drawing a vertical measurement.
   * @param {Cartesian2} mousePosition The mouse position
   */
  VerticalMeasurement.prototype.handleMouseMove = function (mousePosition) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('mousePosition', mousePosition);
      //>>includeEnd('debug');

      if (this._mode !== Mode$1.Drawing) {
          return;
      }

      var label = this._label;
      if (this._scene.mode === cesium.SceneMode.SCENE2D) {
          label.position = this._positions[0];
          label.text = MeasureUnits.distanceToString(0, this._selectedUnits.distanceUnits, this._selectedLocale);
          label.show = true;
          this._mode = Mode$1.AfterDraw;
          return;
      }
      var pos = VerticalMeasurement._getHeightPosition(this, mousePosition);
      if (!cesium.defined(pos)) {
          return;
      }

      var positions = this._positions;
      var pos1 = positions[0];
      var pos2 = positions[1];

      var vec = cesium.Cartesian3.subtract(pos2, pos1, scratch$2);
      var distance = cesium.Cartesian3.magnitude(vec);

      label.position = cesium.Cartesian3.midpoint(pos1, pos2, positionScratch$1);
      label.text = MeasureUnits.distanceToString(distance, this._selectedUnits.distanceUnits, this._selectedLocale);
      label.show = true;

      this._polyline.positions = positions; //triggers polyline update
      this._distance = distance;
  };

  /**
   * Resets the measurement.
   */
  VerticalMeasurement.prototype.reset = function () {
      this._polyline.show = false;
      this._label.show = false;
      this._startPoint.show = false;
      this._endPoint.show = false;
      this._mode = Mode$1.BeforeDraw;
      this._distance = 0;
  };

  /**
   * @returns {Boolean} true if the object has been destroyed, false otherwise.
   */
  VerticalMeasurement.prototype.isDestroyed = function () {
      return false;
  };

  /**
   * Destroys the measurement.
   */
  VerticalMeasurement.prototype.destroy = function () {
      this._primitives.remove(this._polyline);
      var points = this._pointCollection;
      points.remove(this._startPoint);
      points.remove(this._endPoint);
      this._labelCollection.remove(this._label);

      return cesium.destroyObject(this);
  };

  // exposed for specs
  VerticalMeasurement._getWorldPosition = getWorldPosition;
  VerticalMeasurement._getHeightPosition = getHeightPosition;

  function MeasureViewModel(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);

      var scene = options.scene;
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('options.scene', scene);
      //>>includeEnd('debug');

      var units = cesium.defined(options.units) ? options.units : new MeasureUnits();
      var primitives = cesium.defined(options.primitives) ? options.primitives : scene.primitives.add(new cesium.PrimitiveCollection());
      var points = primitives.add(new cesium.PointPrimitiveCollection());
      var labels = primitives.add(new cesium.LabelCollection());

      var mouseHandler = new MeasurementMouseHandler(scene);
      var measurementOptions = {
          scene: scene,
          units: units,
          locale: options.locale,
          points: points,
          labels: labels,
          primitives: primitives
      };
      var componentOptions = cesium.clone(measurementOptions);
      componentOptions.showComponentLines = true;

      var measurements = [new DistanceMeasurement(measurementOptions), new DistanceMeasurement(componentOptions), new PolylineMeasurement(measurementOptions), new HorizontalMeasurement(measurementOptions), new VerticalMeasurement(measurementOptions), new HeightMeasurement(measurementOptions), new AreaMeasurement(measurementOptions), new PointMeasurement(measurementOptions)];

      /**
       * Gets and sets whether the measurement toolbar is expanded.
       * @type {Boolean}
       * @default false
       */
      this.expanded = false;

      /**
       * Gets and sets whether the instructions are visible.
       * @type {Boolean}
       * @default false
       */
      this.instructionsVisible = false;

      /**
       * Gets or sets the currently selected measurement.  This property is observable.
       * @type {Measurement}
       * @default undefined
       */
      this.selectedMeasurement = undefined;
      var selectedMeasurement = cesium.knockout.observable();
      cesium.knockout.defineProperty(this, 'selectedMeasurement', {
          get: function () {
              return selectedMeasurement();
          },
          set: function (value) {
              var old = selectedMeasurement();
              if (cesium.defined(old)) {
                  old.reset();
              }
              selectedMeasurement(value);
              mouseHandler.selectedMeasurement = value;
              if (scene.requestRenderMode) {
                  scene.requestRender();
              }
          }
      });

      cesium.knockout.track(this, ['expanded', 'instructionsVisible']);

      this._measurements = measurements;
      this._units = units;
      this._mouseHandler = mouseHandler;
      this._primitives = primitives;

      this._scene = scene;

      this._removeListener = scene.morphStart.addEventListener(MeasureViewModel.prototype.onMorph, this);
  }

  Object.defineProperties(MeasureViewModel.prototype, {
      /**
       * Gets the scene.
       * @type {Scene}
       * @memberof MeasureViewModel.prototype
       * @readonly
       */
      scene: {
          get: function () {
              return this._scene;
          }
      },
      /**
       * Gets the array of available measurement types.
       * @type {Measurement[]}
       * @memberof MeasureViewModel.prototype
       * @readonly
       */
      measurements: {
          get: function () {
              return this._measurements;
          }
      },
      /**
       * Gets the selected unit of measurement.
       * @type {MeasureUnits}
       * @memberof MeasureViewModel.prototype
       * @readonly
       */
      units: {
          get: function () {
              return this._units;
          }
      }
  });

  /**
   * Toggles the state of the toolbar.
   */
  MeasureViewModel.prototype.toggleActive = function () {
      var expanded = this.expanded;
      if (!expanded) {
          this._activate();
      } else {
          this._deactivate();
      }
      this.expanded = !expanded;
  };

  /**
   * Toggles the visibility of the instructions panel.
   */
  MeasureViewModel.prototype.toggleInstructions = function () {
      this.instructionsVisible = !this.instructionsVisible;
  };

  /**
   * @private
   */
  MeasureViewModel.prototype._activate = function () {
      this._mouseHandler.activate();
      this.selectedMeasurement = this._measurements[0];
  };

  /**
   * @private
   */
  MeasureViewModel.prototype._deactivate = function () {
      this._mouseHandler.deactivate();
      this.selectedMeasurement = undefined;
      this.reset();
  };

  MeasureViewModel.prototype.onMorph = function (transitioner, oldMode, newMode, isMorphing) {
      this.reset();
  };

  /**
   * Resets the widget.
   */
  MeasureViewModel.prototype.reset = function () {
      this.instructionsVisible = false;
      this._measurements.forEach(function (measurement) {
          measurement.reset();
      });
  };

  /**
   * @returns {Boolean} true if the object has been destroyed, false otherwise.
   */
  MeasureViewModel.prototype.isDestroyed = function () {
      return false;
  };

  /**
   * Destroys the widget view model.
   */
  MeasureViewModel.prototype.destroy = function () {
      this._deactivate();
      this._mouseHandler.destroy();
      this._measurements.forEach(function (measurement) {
          measurement.destroy();
      });
      this._scene.primitives.remove(this._primitives);
      return cesium.destroyObject(this);
  };

  function createDomNode(html) {
      //>>includeStart('debug', pragmas.debug);
      cesium.Check.typeOf.string('html', html);
      //>>includeEnd('debug');

      var div = document.createElement('div');
      div.innerHTML = html;

      if (div.children.length === 1) {
          return div.removeChild(div.firstChild);
      }

      return div;
  }

  var html = '<div class="cesium-measure-toolbar" data-bind="css: {expanded: expanded}">\n\
                   <div class="cesium-measure-button cesium-measure-button-main" data-bind="click: toggleActive, attr: {title: expanded ? \'Collapse\' : \'Expand\'}">\n\
                       <svg width="25px" height="25px" viewBox="0 0 30 30">\
                          <path d="M 14.851122,11.545456 25.578177,0.95157924 29.306163,4.6249483 18.537701,15.246448 M 15.097088,18.640104 4.1531358,29.434698 0.62547531,25.653101 11.515909,14.853004"/>\
                          <path d="M 22.983411,26.662767 0.8350882,3.9632787 4.2110211,0.77972226 26.222602,23.217308 Z"/>\
                          <path d="m 23.126668,26.856584 5.906658,2.311965 -2.630951,-5.79394"/>\
                          <path d="M 3.8120758,6.6472825 7.2277612,3.4338416"/>\
                          <path d="M 5.2416124,27.76234 3.6377555,26.08552"/>\
                          <path d="M 7.0148521,26.015847 5.4109952,24.339027"/>\
                          <path d="M 8.9787697,24.078675 7.3749129,22.401855"/>\
                          <path d="M 10.974467,22.173284 9.3706099,20.496464"/>\
                          <path d="m 12.990856,20.120081 -1.603857,-1.67682"/>\
                          <path d="m 19.676091,13.638423 -1.603857,-1.67682"/>\
                          <path d="M 21.449331,11.89193 19.845474,10.21511"/>\
                          <path d="M 23.413248,9.9547586 21.809392,8.2779376"/>\
                          <path d="M 25.448671,7.9858073 23.805089,6.3725466"/>\
                          <path d="M 27.425335,5.9961636 25.765864,4.3670131"/>\
                    </svg>\
               </div>\n\
                   <!-- ko foreach: measurements -->\n\
                   <div class="cesium-measure-button" data-bind="click: function($data) { $parent.selectedMeasurement = $data; }, attr: {title: type}, css: {active: $data === $parent.selectedMeasurement}, html: thumbnail"></div>\n\
                   <!-- /ko -->\n\
                   <div class="cesium-measure-button cesium-measure-help" title="Settings and Help" data-bind="click: toggleInstructions, css: {active: instructionsVisible}">\n\
                       <svg width="20px" height="20px" viewBox="0 0 30 30">\
                            <g transform="translate(0,-267)">\
                            <g>\
                            <path d="M 16.891904,289.31984 H 11.7387 q -0.02021,-1.11147 -0.02021,-1.35398 0,-2.50587 0.828554,-4.12256 0.828555,-1.61669 3.314218,-3.63756 2.485663,-2.02086 2.970671,-2.64733 0.747719,-0.99022 0.747719,-2.18253 0,-1.65711 -1.33377,-2.82921 -1.313562,-1.19231 -3.556721,-1.19231 -2.162325,0 -3.617348,1.23273 -1.4550219,1.23272 -2.0006553,3.7588 l -5.2138301,-0.64667 q 0.2222951,-3.61735 3.0717138,-6.14343 2.8696274,-2.52608 7.5176156,-2.52608 4.890492,0 7.780328,2.5665 2.889836,2.54629 2.889836,5.94134 0,1.8794 -1.071058,3.55672 -1.05085,1.67732 -4.526737,4.56715 -1.798569,1.49544 -2.243159,2.40483 -0.424381,0.90939 -0.383964,3.25359 z m -5.153204,7.63887 v -5.67863 h 5.678629 v 5.67863 z" />\
                            </g>\
                            </g>\
                        </svg>\
                   </div>\n\
               </div>\n\
               <div class="cesium-measure-instructions" data-bind="visible: instructionsVisible">\n\
                   <!-- ko foreach: measurements -->\n\
                   <div><div class="cesium-measure-icon" data-bind="html: icon" style="display: inline"></div><span data-bind="text: type"></span></div>\n\
                   <ul data-bind="foreach: instructions">\n\
                       <li data-bind="text: $data"></li>\n\
                   </ul>\n\
                   <hr>\n\
                   <!-- /ko -->\n\
               </div>';

  function Measure(options) {
      options = cesium.defaultValue(options, cesium.defaultValue.EMPTY_OBJECT);
      var container = options.container;

      //>>includeStart('debug', pragmas.debug);
      cesium.Check.defined('options.container', container);
      cesium.Check.defined('options.scene', options.scene);
      //>>includeEnd('debug');

      var element = createDomNode(html);

      container = getElement(container);
      element = container.appendChild(element);

      var viewModel = new MeasureViewModel(options);

      cesium.knockout.applyBindings(viewModel, container);

      this._viewModel = viewModel;
      this._container = container;
      this._element = element;
      this._dropdown = element.getElementsByClassName('cesium-measure-instructions')[0];
  }

  Object.defineProperties(Measure.prototype, {
      container: {
          get: function () {
              return this._container;
          }
      },

      viewModel: {
          get: function () {
              return this._viewModel;
          }
      },

      tilesetMeasurementSupported: {
          get: function () {
              return this._scene.pickPositionSupported;
          }
      }
  });

  Measure.prototype.isDestroyed = function () {
      return false;
  };

  Measure.prototype.destroy = function () {
      this._viewModel.destroy();
      cesium.knockout.cleanNode(this._container);
      this._container.removeChild(this._element);

      return cesium.destroyObject(this);
  };

  function viewerMeasureMixin(viewer, options) {
      //>>includeStart('debug', pragmas.debug);
      if (!cesium.defined(viewer)) {
          throw new cesium.DeveloperError('viewer is required.');
      }
      //>>includeEnd('debug');

      options = cesium.defaultValue(options, {});
      var scene = viewer.scene;
      var cesiumMeasureContainer = document.createElement('div');
      cesiumMeasureContainer.className = 'cesium-viewer-measureContainer';
      viewer._toolbar.insertBefore(cesiumMeasureContainer, viewer._toolbar.firstChild);
      options = cesium.clone(options);
      options.container = cesiumMeasureContainer;
      options.scene = scene;
      var measure = new Measure(options);

      var removeListener = scene.postUpdate.addEventListener(function () {
          var panelMaxHeight = viewer._container.clientHeight - 125;
          measure._dropdown.style.maxHeight = panelMaxHeight + 'px';
      });

      viewer.destroy = cesium.wrapFunction(viewer, viewer.destroy, function () {
          removeListener();
          measure.destroy();
      });

      Object.defineProperties(viewer, {
          measure: {
              get: function () {
                  return measure;
              }
          }
      });
  }

  window.Cesium.MeasurementMouseHandler = MeasurementMouseHandler;
  window.Cesium.DistanceMeasurement = DistanceMeasurement;
  window.Cesium.PolylineMeasurement = PolylineMeasurement;
  window.Cesium.HorizontalMeasurement = HorizontalMeasurement;
  window.Cesium.VerticalMeasurement = VerticalMeasurement;
  window.Cesium.HeightMeasurement = HeightMeasurement;
  window.Cesium.AreaMeasurement = AreaMeasurement;
  window.Cesium.PointMeasurement = PointMeasurement;

  window.Cesium.MeasureUnits = MeasureUnits;
  window.Cesium.DistanceUnits = DistanceUnits;
  window.Cesium.AreaUnits = AreaUnits;
  window.Cesium.VolumeUnits = AreaUnits;
  window.Cesium.viewerMeasureMixin = viewerMeasureMixin;

})));
